<?php

/**
 * Provide bundles integration with shopping cart and related Views.
 */

/**
 * Implements hook_views_data_alter().
 */
function commerce_bundle_views_data_alter(&$data) {
  // Override the line item title handler
  $data['commerce_line_item']['line_item_title']['field']['handler'] = 'commerce_bundle_handler_field_line_item_title';
  $data['commerce_line_item']['edit_quantity']['field']['handler'] = 'commerce_bundle_handler_field_edit_quantity';
  //dpm($data);
}

/**
 * Implements hook_views_post_execute().
 */
function commerce_bundle_views_post_execute(&$view) {
  if ($view->name == 'commerce_cart_form') {

    // First we need to get the alias for line item id.
    foreach ($view->query->field_aliases as $field_array) {
      if (array_key_exists('line_item_id', $field_array)) {
        $alias = $field_array['line_item_id'];
        break;
      }
    }

    if (is_array($view->result) && !empty($view->result)) {
      // Extract the entities for each result.
      foreach ($view->result as $row_index => $current_row) {
        $current_entity = $current_row->_field_data[$alias]['entity'];
        if ($current_entity->type != 'bundle_product') {
          // We don't care about anything but bundle line items.
          continue;
        }
        $previous_entity = (isset($view->result[$row_index - 1])) ? $view->result[$row_index - 1]->_field_data[$alias]['entity'] : FALSE;

        if (!$previous_entity || $previous_entity->type != 'bundle_product' || $previous_entity->commerce_bundle_id != $current_entity->commerce_bundle_id) {
          // It's  a first bundle line item in the group. Add a control row above the current row.
          $current_entity_wrapper = entity_metadata_wrapper('commerce_line_item', $current_entity);
          $new_row = new stdClass();
          $data =& $new_row->_field_data[$alias];
          $data['entity_type'] = 'bundle_control';
          $data['commerce_bundle_id'] = $current_entity_wrapper->commerce_bundle_id->value();
          $data['commerce_product'] = $current_entity_wrapper->commerce_product->value();
          $data['order_id'] = $current_entity->order_id;
          array_splice($view->result, $row_index, 0, array($new_row));
        }
      }
    }
  }
}
