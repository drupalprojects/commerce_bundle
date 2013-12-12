; Drush API version
api = 2
; Drupal Core version
core = 7.x

; Entity - bundle item module requires 1.2+
projects[entity][type] = module
projects[entity][subdir] = contrib
projects[entity][version] = 1.2

; Commerce
projects[commerce][type] = module
projects[commerce][subdir] = contrib

; Commerce Patches
projects[commerce][patch][] = "https://drupal.org/files/issues/commerce-removes_unused_local_variable-2155333-1_0.patch"

; Entity Reference
projects[entityreference][type] = module
projects[entityreference][subdir] = contrib
