<?php
/**
 * @file
 * Drush commands for maketest module.
 */

/**
 * Implements hook_drush_command().
 */
function id_dup_drush_command() {
  $items['identify'] = array(
    'description' => 'Compares the contents of a makefile to the module list of an existing site to identify potential duplicate modules.',
    'drupal dependencies' => array('system'),
    'arguments' => array(
      'makefile' => "The absolute path to a file, or '-' for STDIN.",
    ),
    'examples' => array(
      'drush identify /path/to/my/makefile' => 'Identify if any modules specified in the makefile already exist in the site directory.',
    ),
  );
  return $items;
}

/*
 * Identifies duplicate modules across the various locations that Drupal allows.
 *
 * @param string $makefile
 *   The path to a file, or '-' for STDIN.
 */
function drush_id_dup_identify($makefile) {
  // Get a list of all modules keyed by uri.
  $modules = drupal_system_listing('/^' . DRUPAL_PHP_FUNCTION_PATTERN . '\.module$/', 'modules', 'uri', 0);
  if (!empty($modules)) {
    // Get the list of modules our makefile adds.
    $info = make_parse_info_file($makefile);
    foreach ($info['projects'] as $name => $value) {
      foreach ($modules as $module) {
        if ($module->name == $name) {
          drush_log(dt("Warning: '@name' already exists at '@uri'", array('@uri' => $module->uri, '@name' => $name)), 'error');
        }
      }
    }
  }
}
