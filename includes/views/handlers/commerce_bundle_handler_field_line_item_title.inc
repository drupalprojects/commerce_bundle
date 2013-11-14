<?php

/**
 * Field handler to return a bundle product line item's or control row's title.
 */
class commerce_bundle_handler_field_line_item_title extends commerce_line_item_handler_field_line_item_title {

  function render($values) {
    // Extract field data containing entity information.
    $alias = $this->aliases['line_item_id'];
    if ($values->_field_data[$alias]['entity_type'] == 'bundle_control') {
      // It's a control row. We return the node display title as the row title.
    }
    elseif ($values->_field_data[$alias]['entity']->type == 'bundle_product') {
      // It's a bundle product line item. Return node display title + product
      // title.
    }
    else {
      return parent::render($values);
    }
    dpm($values->_field_data[$alias]);


  }
}
