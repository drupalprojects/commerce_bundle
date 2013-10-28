<?php

/**
 * @file
 * Defines the inline entity form controller for Commerce Bundle Items.
 */

class CommerceBundleItemInlineEntityFormController extends EntityInlineEntityFormController {
  /**
   * Overrides EntityInlineEntityFormController::tableFields().
   */
  public function tableFields($bundles) {
    $fields = array();

    // If only one type is allowed, its fields can be used as columns.
    if (count($bundles) == 1) {
      $bundle = reset($bundles);

/** @todo: should limit this to only controlled / locked fields ***/
      foreach (field_info_instances($this->entityType, $bundle) as $field_name => $instance) {
        $field = field_info_field($field_name);
        $field_formatter = 'default';
        $field_weight = 0;

        foreach (array('admin_ief', 'default') as $view_mode) {
          if (isset($instance['display'][$view_mode]['type']) && $instance['display'][$view_mode]['type'] != 'hidden') {
            $field_formatter = $instance['display'][$view_mode]['type'];
            $field_weight = $instance['display'][$view_mode]['weight'];
            break;
          }
        }

        $fields[$field_name] = array(
          'type' => 'field',
          'label' => $instance['label'],
          'weight' => $field_weight,
          'formatter' => $field_formatter,
          'delta' => 0,
        );
      }
    }

/*
@todo: always add locked fields once implemented: commerce_price, commerce_product
    if (!isset($fields['commerce_product'])) {
      $fields['commerce_product'] = array(
        'type' => 'field',
        'label' => t('Price'),
        'formatter' => 'commerce_price_formatted_amount',
        'weight' => -99,
      );
    }
    if (!isset($fields['commerce_price'])) {
      $fields['commerce_price'] = array(
        'type' => 'field',
        'label' => t('Price'),
        'formatter' => 'commerce_price_formatted_amount',
        'weight' => -98,
      );
    }
*/
    $fields['status'] = array(
      'type' => 'property',
      'label' => t('Status'),
      'weight' => 100,
    );

    return $fields;
  }

  /**
   * Overrides EntityInlineEntityFormController::defaultSettings().
   */
  public function defaultSettings() {
    $defaults = parent::defaultSettings();
    
    // Deny existing since items lack a native label
    $defaults['allow_existing'] = FALSE;

    return $defaults;
  }

  /**
   * Overrides EntityInlineEntityFormController::settingsForm().
   */
  public function settingsForm($field, $instance) {
    $form = parent::settingsForm($field, $instance);

    // Deny existing since items lack a native label
    $form['allow_existing']['#access'] = FALSE;
    $form['match_operator']['#access'] = FALSE;

    return $form;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityForm().
   */
  public function entityForm($entity_form, &$form_state) {
    global $user;

    // Reference the entity
    $entity = $entity_form['#entity'];

    // Get the labels
    $labels = $this->labels();

    // Get extra fields for form
    $extra_fields = field_info_extra_fields($this->entityType, $entity->type, 'form');

    // Attach fields.
    $langcode = entity_language($this->entityType, $entity);
    if (empty($langcode)) {
      $langcode = LANGUAGE_NONE;
    }
    field_attach_form($this->entityType, $entity, $entity_form, $form_state, $langcode);

    $entity_form['status'] = array(
      '#type' => 'radios',
      '#title' => t('Status'),
      '#default_value' => $entity->status,
      '#options' => array(1 => t('Active'), 0 => t('Disabled')),
      '#required' => TRUE,
      '#weight' => $extra_fields['status']['weight'],
    );

    return $entity_form;
  }

  /**
   * Overrides EntityInlineEntityFormController::delete().
   *
   * Disables entities that can't be deleted (because they are already
   * referenced from a line item, or for some other reason), deletes the rest.
   */
  public function delete($ids, $context) {
    $entities = entity_load($this->entityType, $ids);
    foreach ((array) $entities as $entity_id => $entity) {
      if (!commerce_bundle_item_can_delete($entity)) {
        $entity->status = FALSE;
        entity_save($this->entityType, $entity);

        unset($entities[$entity_id]);
      }
    }

    entity_delete_multiple($this->entityType, array_keys($entities));
  }
}
