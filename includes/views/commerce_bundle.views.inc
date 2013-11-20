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
        $current_entity_wrapper = entity_metadata_wrapper('commerce_line_item', $current_entity);
        $previous_row = isset($view->result[$row_index - 1]) ? $view->result[$row_index - 1] : NULL;
        if ($current_entity->type != 'bundle_product' || (isset($previous_row) && $previous_row->_field_data[$alias]['entity_type'] == 'bundle_control')) {
           // We don't care about anything but bundle line items where the
          // previous row is not a control row.
          continue;
        }
        $previous_entity_wrapper = isset($previous_row) ? entity_metadata_wrapper('commerce_line_item', $previous_row->_field_data[$alias]['entity']) : NULL;

        if (!isset($previous_entity_wrapper) || $previous_row->_field_data[$alias]['entity']->type != 'bundle_product' || $previous_entity_wrapper->commerce_bundle_id->nid->value() != $current_entity_wrapper->commerce_bundle_id->nid->value()) {
          // It's  a first bundle line item in the group. Add a control row
          // above the current row.
          $new_row = new stdClass();
          $new_row->_field_data[$alias]['entity_type'] = 'bundle_control';
          $new_row->_field_data[$alias]['entity'] = new stdClass();
          // Set the display path to the current row's.
          $new_row->field_commerce_display_path = $current_row->field_commerce_display_path;
          $data =& $new_row->_field_data[$alias]['entity'];
          $data->commerce_bundle_id = $current_entity_wrapper->commerce_bundle_id->value();
          $data->commerce_product = $current_entity_wrapper->commerce_product->value();
          $data->order_id = $current_entity->order_id;
          array_splice($view->result, $row_index, 0, array($new_row));
        }
      }
    }
  }
}
