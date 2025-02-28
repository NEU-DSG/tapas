<?php

/**
 * @file
 * Admin forms and pages.
 */

/**
 * Main Authcache configuration form.
 */
function authcache_debug_admin($form, &$form_state) {
  $debug_all     = variable_get('authcache_debug_all', 0);
  $debug_users   = implode(', ', variable_get('authcache_debug_users', array()));
  $dev_query     = variable_get('dev_query', 0);
  $debug_page    = variable_get('authcache_debug_page', 0);
  $cache_periods = array(0, 60, 180, 300, 600, 900, 1800, 2700, 3600, 10800,
    21600, 32400, 43200, 86400);

  $form['debug'] = array(
    '#type' => 'fieldset',
    '#title' => t('Authcache Debugging/Development'),
    '#description' => t("Debug mode will display the page's cache statistics, benchmarks, and Ajax calls."),
    '#collapsible' => TRUE,
  );
  $form['debug']['debug_all'] = array(
    '#title' => t('Enable debug mode for all roles.'),
    '#type' => 'checkbox',
    '#default_value' => $debug_all,
  );
  $form['debug']['debug_users'] = array(
    '#title' => t('Enable for specified users'),
    '#type' => 'textfield',
    '#description' => t('Enter a comma-delimited list of usernames to enable debug mode for.'),
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => $debug_users,
  );
  $period = drupal_map_assoc($cache_periods, 'format_interval');
  $period[0] = t('Purge during cron runs');
  $form['debug']['debug_cache_lifetime'] = array(
    '#type' => 'select',
    '#title' => t('Minimum lifetime for authcache debug information'),
    '#default_value' => variable_get('authcache_debug_cache_lifetime', 0),
    '#options' => $period,
    '#description' => t('Debug information for individual page requests will not be deleted until at least this much time has elapsed.'),
  );
  $form['debug']['dev_query'] = array(
    '#title' => t('Benchmark database queries'),
    '#type' => 'checkbox',
    '#description' => t("This will display the number of queries used, query time, and the percentage related to the page's total render time."),
    '#default_value' => $dev_query,
  );
  $form['debug']['debug_page'] = array(
    '#title' => t('Disable saving pages to cache, but still serve "cached" pages'),
    '#type' => 'checkbox',
    '#description' => t('This prevents pages from being saved to the cache, but renders pages the same as if they were cached. This is useful during development if you find yourself manually clearing the cache too often.'),
    '#default_value' => $debug_page,
  );
  $form['debug']['debug_watchdog'] = array(
    '#title' => t('Log to watchdog'),
    '#type' => 'checkbox',
    '#description' => t('Create a watchdog entry whenever a page is excluded, when page caching was cancelled or when an upcomming page request for a given session will be prevented (preclusion)'),
    '#default_value' => variable_get('authcache_debug_watchdog', FALSE),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Authcache config form submit.
 */
function authcache_debug_admin_submit($form, &$form_state) {
  $debug_user_ray = array();

  // Debugging for users.
  $debug_users = explode(',', $form_state['values']['debug_users']);
  foreach ($debug_users as $username) {
    $debug_user_ray[] = trim($username);
  }
  $form_state['values']['debug_users'] = $debug_user_ray;

  // Devel query logging.
  variable_set('dev_query', $form_state['values']['dev_query']);

  // Delete variable if not in use.
  $varnames = array(
    'debug_all',
    'debug_page',
    'debug_users',
    'debug_cache_lifetime',
    'debug_watchdog',
  );
  foreach ($varnames as $key) {
    if ($value = $form_state['values'][$key]) {
      variable_set("authcache_{$key}", $value);
    }
    else {
      variable_del("authcache_{$key}");
    }
  }

  drupal_set_message(t('Your settings have been saved.'));
}
