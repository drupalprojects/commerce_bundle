<?php

/*
 * Implements hook_default_rules_configuration().
 */
function commerce_bundle_default_rules_configuration() {
  
  // Reaction rule: set the bundle line item price.
  $rule = rules_reaction_rule();

  $rule->label = t('Set bundle price');
  $rule->active = TRUE;
  $rule->weight = -10;

  $rule
    ->event('commerce_product_calculate_sell_price')
    ->condition('commerce_bundle_line_item_type_is_bundle', array(
      'commerce_line_item:select' => 'commerce-line-item'
    ))
    ->action('commerce_bundle_provide_bundle_line_item_amount', array(
      'commerce_line_item:select' => 'commerce-line-item',
      'bundle_line_item_amount:label' => t('Bundle line item amount'),
      'bundle_line_item_amount:var' => 'bundle_line_item_amount'
    ))
    ->action('commerce_line_item_unit_price_amount', array(
      'commerce_line_item:select' => 'commerce-line-item',
      'amount:select' => 'bundle_line_item_amount',
      'component_name' => 'base_price',
      'round_mode' => COMMERCE_ROUND_NONE
    ));

  $rules['commerce_bundle_set_bundle_price'] = $rule;

  return $rules;
}
