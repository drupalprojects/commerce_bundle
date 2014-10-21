<?php

/*
 * Implements hook_rules_action_info().
 */
function commerce_bundle_rules_action_info() {
  $actions['commerce_bundle_provide_bundle_line_item_amount'] = array(
    'label' => t('Provide the unit price amount for a bundle line item'),
    'group' => t('Commerce bundle'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Line item'),
        'wrapped' => TRUE
      )
    ),
    'provides' => array(
      'bundle_line_item_amount' => array(
        'type' => 'decimal',
        'label' => t('Bundle line item amount')
      )
    ),
    'callbacks' => array(
      'execute' => 'commerce_bundle_rules_provide_bundle_line_item_amount'
    )
  );

  return $actions;
}

/*
 * Rules callback: provide the correct price for a bundle line item.
 */
function commerce_bundle_rules_provide_bundle_line_item_amount($line_item_wrapper) {
  $amount = 0;

  // Do nothing if the line item is not provisioned for bundles.
  if ($line_item_wrapper->commerce_bundle_group_id->value() && in_array($line_item_wrapper->type->value(), commerce_bundle_line_item_types())) {
    // Get the proper bundle price.
    $price = commerce_bundle_get_bundle_product_price($line_item_wrapper->commerce_bundle_group_id->raw(), $line_item_wrapper->commerce_bundle_item_id->raw());
    $amount = $price['amount'];
  }

  return array('bundle_line_item_amount' => $amount);
}
