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
  dpm($data);
}
