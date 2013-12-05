<?php

/**
 * @file
 * Provides a field formatter for bundle add to cart
 */


/**
 * Implements hook_field_formatter_info().
 */
function commerce_bundle_field_formatter_info() {
  return array(
    'commerce_bundle_add_to_cart_form' => array(
      'label' => t('Bundle Add to Cart form'),
      'description' => t('Display an Add to Cart form for the referenced bundle product groups.'),
      'field types' => array('commerce_product_reference'),
      'settings' => array(
        'show_quantity' => FALSE,
        'default_quantity' => 1,
        'combine' => TRUE,
        'show_single_product_attributes' => FALSE,
        'line_item_type' => 'product',
      ),
    ),
  );
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function commerce_bundle_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'] + field_info_formatter_settings($display['type']);

  $element = array();

  if ($display['type'] == 'commerce_bundle_add_to_cart_form') {
    $element['show_quantity'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display a textfield quantity widget on the add to cart form.'),
      '#default_value' => $settings['show_quantity'],
    );

    $element['default_quantity'] = array(
      '#type' => 'textfield',
      '#title' => t('Default quantity'),
      '#default_value' => $settings['default_quantity'] <= 0 ? 1 : $settings['default_quantity'],
      '#element_validate' => array('commerce_cart_field_formatter_settings_form_quantity_validate'),
      '#size' => 16,
    );

    $element['combine'] = array(
      '#type' => 'checkbox',
      '#title' => t('Attempt to combine like products on the same line item in the cart.'),
      '#description' => t('The line item type, referenced product, and data from fields exposed on the Add to Cart form must all match to combine.'),
      '#default_value' => $settings['combine'],
    );

    $element['show_single_product_attributes'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show attribute widgets even if the Add to Cart form only represents one product.'),
      '#description' => t('If enabled, attribute widgets will be shown on the form with the only available options selected.'),
      '#default_value' => $settings['show_single_product_attributes'],
    );

    // Add a conditionally visible line item type element.
    $line_item_types = commerce_bundle_line_item_types();

    if (count($line_item_types) > 1) {
      $element['line_item_type'] = array(
        '#type' => 'select',
        '#title' => t('Add to Cart line item type'),
        '#options' => array_intersect_key(commerce_line_item_type_get_name(), drupal_map_assoc($line_item_types)),
        '#default_value' => $settings['line_item_type'],
      );
    }
    else {
      $element['line_item_type'] = array(
        '#type' => 'hidden',
        '#value' => key($line_item_types),
      );
    }
  }

  return $element;
}

/**
 * Element validate callback: ensure a valid quantity is entered.
 */
function commerce_bundle_field_formatter_settings_form_quantity_validate($element, &$form_state, $form) {
  if (!is_numeric($element['#value']) || $element['#value'] <= 0) {
    form_set_error(implode('][', $element['#parents']), t('You must enter a positive numeric default quantity value.'));
  }
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function commerce_bundle_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'] + field_info_formatter_settings($display['type']);

  $summary = array();

  if ($display['type'] == 'commerce_bundle_add_to_cart_form') {
    $summary = array(
      t('Quantity widget: !status', array('!status' => !empty($settings['show_quantity']) ? t('Enabled') : t('Disabled'))),
      t('Default quantity: @quantity', array('@quantity' => $settings['default_quantity'])),
      t('Combine like items: !status', array('!status' => !empty($settings['combine']) ? t('Enabled') : t('Disabled'))),
      t('!visibility attributes on single product forms.', array('!visibility' => !empty($settings['show_single_product_attributes']) ? t('Showing') : t('Hiding'))),
    );

    if (count(commerce_bundle_line_item_types()) > 1) {
      $summary[] = t('Add to Cart line item type: @type', array('@type' => commerce_line_item_type_get_name($settings['line_item_type'])));
    }
  }

  return implode('<br />', $summary);
}

/**
 * Implements hook_field_formatter_view().
 */
function commerce_bundle_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $settings = $display['settings'] + field_info_formatter_settings($display['type']);
  $result = array();

  // Collect the list of product group IDs.
  $group_ids = array();

  foreach ($items as $delta => $item) {
    $group_ids[] = $item['product_id'];
  }

  // Exit early if no references.
  if (empty($group_ids)) {
    return $result;
  }

  if ($display['type'] == 'commerce_bundle_add_to_cart_form') {
    // Load the referenced product groups.
    $groups = commerce_product_load_multiple($group_ids);

    // Check to ensure products are referenced, before returning results.
    if (!empty($groups)) {
      $type = !empty($settings['line_item_type']) ? $settings['line_item_type'] : 'bundle_product';
      $bundle_item_field_name = 'commerce_bundle_item_id';

      // Create a line item per group.
      $line_items = array();
      foreach ($groups as $group_id => $group) {
        if ($bundle_item = commerce_bundle_item_field_get_default_item('commerce_product', $group, $bundle_item_field_name)) {
          $line_item = commerce_bundle_product_line_item_new($bundle_item, $group, $entity, $settings['default_quantity'], 0, array(), $type);

          // @todo: do we still need to add bundle_item ids if group is
          // referenced on the line item.
          // $line_item->data['context']['product_ids'] = array_keys($products);

          $line_item->data['context']['add_to_cart_combine'] = !empty($settings['combine']);
          $line_item->data['context']['show_single_product_attributes'] = !empty($settings['show_single_product_attributes']);

          // Store line item per group id.
          $line_items[$group->product_id] = $line_item;
        }
      }

      $result[] = array(
        '#arguments' => array(
          'form_id' => commerce_bundle_add_to_cart_form_id($group_ids),
          'line_items' => $line_items,
          'show_quantity' => $settings['show_quantity'],
        ),
      );
    }
  }

  return $result;
}

/**
 * Implements hook_field_attach_view_alter().
 *
 * When a field is formatted for display, the display formatter does not know
 * what view mode it is being displayed for. Unfortunately, the Add to Cart form
 * display formatter needs this information when displaying product reference
 * fields on nodes to provide adequate context for product field replacement on
 * multi-value product reference fields. This hook is used to transform a set of
 * arguments into a form using the arguments and the extra context information
 * gleaned from the parameters passed into this function.
 */
function commerce_bundle_field_attach_view_alter(&$output, $context) {
  // Loop through the fields passed in looking for any product reference fields
  // formatted with the Add to Cart form display formatter.
  foreach ($output as $field_name => $element) {
    if (!empty($element['#formatter']) && $element['#formatter'] == 'commerce_bundle_add_to_cart_form') {
      // Prepare the context information needed by the cart form.
      $cart_context = $context;

      // Remove the full entity from the context array and put the ID in instead.
      list($entity_id, $vid, $bundle) = entity_extract_ids($context['entity_type'], $context['entity']);
      $cart_context['entity_id'] = $entity_id;
      unset($cart_context['entity']);

      // Remove any Views data added to the context by views_handler_field_field.
      // It unnecessarily increases the size of rows in the cache_form table for
      // Add to Cart form state data.
      if (!empty($cart_context['display']['views_view'])) {
        unset($cart_context['display']['views_view']);
        unset($cart_context['display']['views_field']);
        unset($cart_context['display']['views_row_id']);
      }

      // Add the context for displaying product fields in the context of an entity
      // that references the product by looking at the entity this product
      // reference field is attached to.
      $cart_context['class_prefix'] = $context['entity_type'] . '-' . $entity_id;
      $cart_context['view_mode'] = $context['entity_type'] . '_' . $element['#view_mode'];

      $entity_uri = entity_uri($context['entity_type'], $element['#object']);

      foreach (element_children($element) as $key) {
        // Extract the drupal_get_form() arguments array from the element.
        $arguments = $element[$key]['#arguments'];

        foreach ($arguments['line_items'] as &$line_item) {
          // Add the display path and referencing entity data to the line item.
          if (!empty($entity_uri['path'])) {
            $line_item->data['context']['display_path'] = $entity_uri['path'];
          }

          $line_item->data['context']['entity'] = array(
            'entity_type' => $context['entity_type'],
            'entity_id' => $entity_id,
            'product_reference_field_name' => $field_name,
          );

          // Always point to the entity data since line item already has a
          // product group reference and bundle item reference.
          $line_item->data['context']['product_ids'] = 'entity';
        }
        unset($line_item);

        // Replace the array containing the arguments with the return value of
        // drupal_get_form(). It will be rendered when the rest of the object is
        // rendered for display.
        $output[$field_name][$key] = drupal_get_form($arguments['form_id'], $arguments['line_items'], $arguments['show_quantity'], $cart_context);
      }
    }
  }
}
