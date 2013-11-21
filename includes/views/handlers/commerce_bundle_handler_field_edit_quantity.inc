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
        // of the sum of the unit quantities for the product groups that make
        // up the bundle.
        $unit_qtys = db_query("SELECT commerce_bundle_unit_quantity_value as unit_qty, commerce_bundle_group_id_target_id as group_id FROM {commerce_line_item} cli
                                JOIN {field_data_commerce_bundle_id bid} ON cli.line_item_id = bid.entity_id
                                JOIN {field_data_commerce_bundle_unit_quantity} uq ON cli.line_item_id = uq.entity_id
                                JOIN {field_data_commerce_bundle_group_id} gid on cli.line_item_id = gid.entity_id
                                WHERE order_id = :data_order_id AND commerce_bundle_id_target_id = :commerce_bundle_id
                                GROUP BY  commerce_bundle_group_id_target_id", array(':data_order_id' => $data['entity']->order_id, ':commerce_bundle_id' => $data['entity']->commerce_bundle_id->nid))
                                ->fetchAllAssoc('group_id', PDO::FETCH_ASSOC);
        $unit_qty_sum = 0;
        foreach($unit_qtys as $result) {
          $unit_qty_sum += $result['unit_qty'];
        }

        $order_qty_sum = db_query("SELECT SUM(quantity) FROM commerce_line_item cli
                                    JOIN field_data_commerce_bundle_id bid ON cli.line_item_id = bid.entity_id
                                    WHERE order_id = :data_order_id AND commerce_bundle_id_target_id = :commerce_bundle_id",
                                    array(':data_order_id' => $data['entity']->order_id, ':commerce_bundle_id' => $data['entity']->commerce_bundle_id->nid))
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
}
