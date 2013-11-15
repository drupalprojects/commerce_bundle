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
      if ($row->_field_data[$alias]['entity_type'] != 'bundle_control') {
        // It's either a bundle product line item, or a regular line item.
        $line_item_id = $this->get_value($row, 'line_item_id');
        $quantity = $this->get_value($row, 'quantity');

        $form[$this->options['id']][$row_id]['#default_value'] = round($quantity);
        $form[$this->options['id']][$row_id]['#maxlength'] = max(4, strlen($quantity));
        $form[$this->options['id']][$row_id]['#line_item_id'] = $line_item_id;

        if($row->_field_data[$alias]['entity']->type == 'bundle_product') {
          // It's a bundle product line item. Disable it.
          $form[$this->options['id']][$row_id]['#disabled'] = TRUE;
        }
      }
      else {
        dpm('I should get hit once!');
      }
    }
  }
}
