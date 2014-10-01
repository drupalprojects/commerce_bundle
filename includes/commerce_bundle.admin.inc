<?php

/*
 * Form callback: bundle settings
 */
function commerce_bundle_settings_form() {
  $form = array();

  $form['bundle_line_item_types'] = array(
    '#title' => t('Bundle line item types'),
    '#type' => 'checkboxes',
    '#description' => t('Specify which line item types can be used by product bundle groups.'),
    '#options' => commerce_line_item_type_options_list(),
    '#default_value' => commerce_bundle_line_item_types()
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#suffix' => '<span>Pressing save will clear the cache and rebuild field definitions.</span>'
  );

  return $form;
}

/*
 * Form submit handler: save bundle settings variables and clear cache.
 */
function commerce_bundle_settings_form_submit(&$form, &$form_state) {
  variable_set('bundle_line_item_types', $form_state['values']['bundle_line_item_types']);

  // Clear caches to trigger field rebuiild.
  drupal_flush_all_caches();
}
