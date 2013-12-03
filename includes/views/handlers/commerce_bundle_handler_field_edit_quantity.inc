<?php

/**
 * Field handler for bundle line item quantity integration.
 */
class commerce_bundle_handler_field_edit_quantity extends commerce_line_item_handler_field_edit_quantity {

  /**
   * Returns the form which replaces the placeholder from render().
   */
  function views_form(&$form, &$form_state) {
    // The view is empty, abort.
    if (empty($this->view->result)) {
      return;
    }
    $form[$this->options['id']] = array(
      '#tree' => TRUE,
    );
    // At this point, the query has already been run, so we can access the results
    // in order to get the base key value (for example, nid for nodes).
    foreach ($this->view->result as $row_id => $row) {
      // Bundle product line items and control rows get a quantity element, i.e.
      // all rows get an element.
      $form[$this->options['id']][$row_id] = array(
        '#type' => 'textfield',
        '#datatype' => 'integer',
        '#size' => 4,
        '#attributes' => array(
          'title' => $this->options['label'],
        ),
      );

      // Extract field data containing entity information.
      $alias = $this->aliases['line_item_id'];
      $data =& $row->_field_data[$alias];
      $form_data =& $form[$this->options['id']][$row_id];
      if ($data['entity_type'] == 'bundle_control') {
        // It's a bundle control row. The bundle quantity will be a multiple
        // of the sum of the unit quantities for line items that have the same
        // bundle_config_id as the control row.

        $unit_qty_sum = db_query("SELECT SUM(commerce_bundle_unit_quantity_value) as unit_qty FROM {field_data_commerce_bundle_unit_quantity} uq
                                JOIN {field_data_commerce_bundle_config_id cid} ON uq.entity_id = cid.entity_id
                                WHERE commerce_bundle_config_id_value = :commerce_bundle_config_id",
                                array(':commerce_bundle_config_id' => $data['entity']->commerce_bundle_config_id))
                                ->fetchField();

        $order_qty_sum = db_query("SELECT SUM(quantity) FROM commerce_line_item cli
                                    JOIN {field_data_commerce_bundle_config_id cid} ON cli.line_item_id = cid.entity_id
                                    WHERE order_id = :data_order_id AND commerce_bundle_config_id_value = :commerce_bundle_config_id",
                                    array(':data_order_id' => $data['entity']->order_id, ':commerce_bundle_config_id' => $data['entity']->commerce_bundle_config_id))
                                    ->fetchField();

        $form_data['#default_value'] = round($order_qty_sum/$unit_qty_sum);
      }
      else {
        // It's either a bundle product line item, or a regular line item.
        $line_item_id = $this->get_value($row, 'line_item_id');
        $quantity = $this->get_value($row, 'quantity');
        $form_data['#default_value'] = round($quantity);
        $form_data['#maxlength'] = max(4, strlen($quantity));
        $form_data['#line_item_id'] = $line_item_id;

        if($data['entity']->type == 'bundle_product') {
          // It's a bundle product line item. Disable it.
          $form_data['#disabled'] = TRUE;
        }
      }
    }
  }

  function views_form_submit($form, &$form_state) {
    $field_name = $this->options['id'];
    $deleted_line_items = array();
    $updated_line_items = array();
    $bundle_row_indices = array();

    $alias = $this->aliases['line_item_id'];

    foreach (element_children($form[$field_name]) as $row_id) {
      $row_data = $this->view->result[$row_id]->_field_data[$alias];
      if ($row_data['entity_type'] == 'bundle_control') {
        // It's a bundle control row. Grab all the row indices that are part
        // of this bundle.
        foreach ($this->view->result as $row_index => $data) {
          if ($data->_field_data[$alias]['entity_type'] != 'bundle_control') {
            $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $data->_field_data[$alias]['entity']);
            if (isset($line_item_wrapper->commerce_bundle_id) && $row_data['entity']->commerce_bundle_id->nid == $line_item_wrapper->commerce_bundle_id->nid->value()) {
              $bundle_row_indices[] = $row_index;
            }
          }
        }
        // Set the form_state quantity value for each bundle line item, then
        // let the default commerce code do it's work.
        foreach ($bundle_row_indices as $index) {
          $eff_unit_qty = $this->commerce_bundle_calculate_effective_unit_qty($this->view->result[$index], $alias);
          // Bundle line item quantity is control row qty x effective unit qty.
          $form_state['values'][$field_name][$index] = $form_state['values'][$field_name][$row_id]*$eff_unit_qty;
        }
        continue;
      }

      $line_item_id = $form[$field_name][$row_id]['#line_item_id'];

      // If the line item hasn't been deleted...
      if ($line_item = commerce_line_item_load($line_item_id)) {
        $form_quantity = $form_state['values'][$field_name][$row_id];

        // If the quantity on the form is different...
        if ($form_quantity != $line_item->quantity) {
          // If the quantity specified is 0, flag the line item for deletion.
          if ($form_quantity == 0) {
            $deleted_line_items[] = $line_item_id;
          }
          else {
            // Otherwise queue the line item quantity update.
            $updated_line_items[$line_item_id] = $form_quantity;
          }
        }
      }
    }

    // Process the deletes first.
    foreach ($deleted_line_items as $line_item_id) {
      $order = commerce_order_load($form_state['order']->order_id);
      commerce_cart_order_product_line_item_delete($order, $line_item_id);
    }

    // Then process the quantity updates.
    foreach ($updated_line_items as $line_item_id => $quantity) {
      // Load the line item and update it.
      $line_item = commerce_line_item_load($line_item_id);
      $line_item->quantity = $quantity;
      commerce_line_item_save($line_item);
      entity_get_controller('commerce_line_item')->resetCache(array($line_item->line_item_id));
    }
  }

  /**
   * Normalizes the unit quantity based on number of line items in cart.
   *
   * @param object $row
   *  The views row for a bundle line item.
   *
   * @param string $alias
   *   Alias for the line item id field.
   *
   * @return
   *   The normalized unit quantity for a particular line item.
   */
  function commerce_bundle_calculate_effective_unit_qty($row, $alias) {

    $entity_wrapper = entity_metadata_wrapper('commerce_line_item', $row->_field_data[$alias]['entity']);
    $group_unit_qty = $entity_wrapper->commerce_bundle_group_id->commerce_bundle_unit_quantity->value();
    // This is the number of line items in same order, bundle, and
    // group, as the $row line item.
    $bundle_group_sum = db_query("SELECT COUNT(line_item_id) FROM commerce_line_item cli
                                  JOIN field_data_commerce_bundle_id bid ON cli.line_item_id = bid.entity_id
                                  JOIN field_data_commerce_bundle_group_id gid ON cli.line_item_id = gid.entity_id
                                  WHERE order_id = :order_id AND commerce_bundle_id_target_id = :commerce_bundle_id AND commerce_bundle_group_id_target_id = :commerce_bundle_group_id",
                                  array(':order_id' => $entity_wrapper->order_id->value(), ':commerce_bundle_id' => $entity_wrapper->commerce_bundle_id->nid->value(),
                                        ':commerce_bundle_group_id' => $entity_wrapper->commerce_bundle_group_id->product_id->value()))
                                  ->fetchField();

    return round($group_unit_qty/$bundle_group_sum);
  }
}
