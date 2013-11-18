<?php

/**
 * Field handler for bundle line item title integration.
 */
class commerce_bundle_handler_field_line_item_title extends commerce_line_item_handler_field_line_item_title {

  function render($values) {
    // Extract field data containing entity information.
    $alias = $this->aliases['line_item_id'];
    $data =& $values->_field_data[$alias];
    if ($data['entity_type'] == 'bundle_control') {
      // It's a control row. We return the node display title as the row title.
      return $data['commerce_bundle_id']->title;
    }
    elseif ($data['entity']->type == 'bundle_product') {
      // It's a bundle product line item. Return node display title + product
      // title.
      $entity_wrapper = entity_metadata_wrapper('commerce_line_item', $data['entity']);
      $node_title = $entity_wrapper->commerce_bundle_id->title->value();
      $product_title = $entity_wrapper->commerce_product->title->value();
      return $product_title . '--' . $node_title;
    }
    else {
      return parent::render($values);
    }
  }
}
