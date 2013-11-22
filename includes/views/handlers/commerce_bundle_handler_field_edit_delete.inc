<?php

/**
 * @file
 * Field handler to present a button to remove a line item or a bundle of line
 * items
 */


class commerce_bundle_handler_field_edit_delete extends commerce_line_item_handler_field_edit_delete {

  /**
   * Returns the form which replaces the placeholder from render().
   */
  function views_form(&$form, &$form_state) {
    // The view is empty, abort.
    if (empty($this->view->result)) {
      return;
    }

    $form[$this->options['id']] = array(
      '#tree' => TRUE,
    );

    // At this point, the query has already been run, so we can access the results
    // in order to get the base key value (for example, nid for nodes).
    foreach ($this->view->result as $row_id => $row) {

      // Extract field data containing entity information.
      $alias = $this->aliases['line_item_id'];
      $data =& $row->_field_data[$alias];
      $form_data =& $form[$this->options['id']][$row_id];
      if (isset($data['entity']->type) && $data['entity']->type == 'bundle_product') {
        // Only the control row of a bundle gets a 'Remove' button.
        continue;
      }

      $line_item_id = $this->get_value($row);

      $form[$this->options['id']][$row_id] = array(
        '#type' => 'submit',
        '#value' => t('Delete'),
        '#name' => 'delete-line-item-' . $row_id,
        '#attributes' => array('class' => array('delete-line-item')),
        '#line_item_id' => $line_item_id,
        '#submit' => array_merge($form['#submit'], array('commerce_line_item_line_item_views_delete_form_submit')),
      );
    }
  }

}
