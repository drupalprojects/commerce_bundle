<?php

/*
 * Implements hook_default_rules_configuration().
 */
function commerce_bundle_default_rules_configuration() {
  // Rule component: set a bundle line item price for bundle line items.
  $variables = array(
    'amount' => array(
      'type' => 'decimal',
      'label' => t('Amount')
    ),
    'commerce_line_item' => array(
      'type' => 'commerce_line_item',
      'label' => t('Line item')
    )
  );

  $rule = rule($variables);

  $rule->label = t('Set bundle price amount');
  $rule->active = TRUE;

  $rule
    ->condition('data_is', array(
      'data:select' => 'amount',
      'op' => '>',
      'value' => '0'
    ))
    ->action('commerce_line_item_unit_price_amount', array(
      'commerce_line_item:select' => 'commerce-line-item',
      'amount:select' => 'amount',
      'component_name' => 'base_price',
      'round_mode' => COMMERCE_ROUND_NONE
    ));

  $rules['commerce_bundle_set_bundle_price_amount'] = $rule;

  // Reaction rule: set the bundle line item price.
  $rule = rules_reaction_rule();

  $rule->label = t('Set bundle price');
  $rule->active = TRUE;
  $rule->weight = -10;

  $rule
    ->event('commerce_product_calculate_sell_price')
    ->action('commerce_bundle_provide_bundle_line_item_amount', array(
      'commerce_line_item:select' => 'commerce-line-item',
      'bundle_line_item_amount:label' => t('Bundle line item amount'),
      'bundle_line_item_amount:var' => 'bundle_line_item_amount'
    ))
    ->action('component_commerce_bundle_set_bundle_price_amount', array(
      'commerce_line_item:select' => 'commerce-line-item',
      'amount:select' => 'bundle_line_item_amount'
    ));

  $rules['commerce_bundle_set_bundle_price'] = $rule;

  return $rules;
}
