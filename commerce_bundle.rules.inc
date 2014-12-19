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
 * Implements hook_rules_condition_info().
 */
function commerce_bundle_rules_condition_info() {
  $conditions['commerce_bundle_line_item_type_is_bundle'] = array(
    'label' => t('Line item can be used in product bundles'),
    'group' => t('Commerce bundle'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Line item'),
        'wrapped' => TRUE
      )
    ),
    'callbacks' => array(
      'execute' => 'commerce_bundle_rules_line_item_is_bundle'
    )
  );

  return $conditions;
}

/*
 * Rules condition callback: determine whether a line item has been provisioned
 * for bundles.
 */
function commerce_bundle_rules_line_item_is_bundle($line_item_wrapper) {
  return $line_item_wrapper->__isset('commerce_bundle_group_id') && $line_item_wrapper->commerce_bundle_group_id->value() && in_array($line_item_wrapper->type->value(), commerce_bundle_line_item_types());
}

/*
 * Rules callback: provide the correct price for a bundle line item.
 */
function commerce_bundle_rules_provide_bundle_line_item_amount($line_item_wrapper) {
  // Get the proper bundle price.
  $price = commerce_bundle_get_bundle_product_price($line_item_wrapper->commerce_bundle_group_id->raw(), $line_item_wrapper->commerce_bundle_item_id->raw());
  $amount = $price['amount'];

  return array('bundle_line_item_amount' => $amount);
}
