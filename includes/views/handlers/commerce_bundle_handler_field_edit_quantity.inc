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
      if ($data['entity_type'] != 'bundle_control') {
        // It's either a bundle product line item, or a regular line item.
        $line_item_id = $this->get_value($row, 'line_item_id');
        $quantity = $this->get_value($row, 'quantity');
        $form_data =& $form[$this->options['id']][$row_id];
        $form_data['#default_value'] = round($quantity);
        $form_data['#maxlength'] = max(4, strlen($quantity));
        $form_data['#line_item_id'] = $line_item_id;

        if($data['entity']->type == 'bundle_product') {
          // It's a bundle product line item. Disable it.
          $form_data['#disabled'] = TRUE;
        }
      }
      else {
        // It's a bundle control row. The bundle quantity will be a multiple
        // of the sum of the unit quantities for the product groups that make
        // up the bundle.
        $order_qty_sum = 0;
        $order = commerce_order_load($data['order_id']);
        $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
        foreach ($order_wrapper->commerce_line_items as $line_item) {
         // dpm($line_item->value(), '$line_item');
         // dpm(isset($line_item->commerce_bundle_id), 'isset');
          if (isset($line_item->commerce_bundle_id) && $line_item->commerce_bundle_id->nid->value() == $data['commerce_bundle_id']->nid) {
            $order_qty_sum += $line_item->quantity->value();
          }
        }
      }
    }
  }
}
