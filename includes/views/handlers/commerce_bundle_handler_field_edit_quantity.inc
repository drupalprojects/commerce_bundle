<?php

/**
 * Field handler for bundle line item quantity integration.
 */
class commerce_bundle_handler_field_edit_quantity extends commerce_line_item_handler_field_edit_quantity {

  /**
   * Returns the form which replaces the placeholder from render().
   */
  function views_form(&$form, &$form_state) {
    dpm($form, '$form');
    dpm($form_state, '$form_state');
  }
}
