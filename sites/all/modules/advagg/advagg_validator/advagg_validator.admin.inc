<?php

/**
 * @file
 * Admin page callbacks for the advagg validator module.
 */

/**
 * Form builder; validator advagg.
 */
function advagg_validator_admin_settings_form() {
  drupal_set_title(t('AdvAgg: Validator'));
  $config_path = advagg_admin_config_root_path();

  $form = array();
  module_load_include('inc', 'advagg_validator', 'advagg_validator');

  $form['per_folder_css_checking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Per Folder CSS Checking'),
    '#description' => t('Can check CSS files, even if not used inside of AdvAgg.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $files = advagg_validator_scan_all_dirs('css');
  sort($files);
  foreach ($files as $file) {
    $dir = dirname($file);
    if (!isset($form['per_folder_css_checking'][$dir])) {
      $form['per_folder_css_checking'][$dir] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($dir),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form_api_dirname = str_replace(array('/', '.'), array('__', '--'), $dir);
      $wrapper = 'advagg-validator-css-validator-ajax' . $form_api_dirname;

      $form['per_folder_css_checking'][$dir]['button_' . $dir] = array(
        '#type' => 'submit',
        '#value' => t('Check this Directory: @dirname', array('@dirname' => $dir)),
        '#submit' => array('advagg_validator_admin_test_advagg_css_directory_submit'),
        "#ajax" => array(
          "callback" => "advagg_validator_admin_test_advagg_css_directory_callback",
          "wrapper" => $wrapper,
          "effect" => 'fade',
        ),
      );
      $form['per_folder_css_checking'][$dir]["wrapper"] = array(
        "#markup" => "<div id='" . $wrapper . "'></div>",
      );
    }
    $form_api_filename = str_replace(array('/', '.'), array('__', '--'), $file);
    $form['per_folder_css_checking'][$dir][$form_api_filename] = array(
      '#markup' => check_plain($file) . ' ',
    );
    $form['per_folder_css_checking'][$dir]['hidden_' . $form_api_filename] = array(
      '#type' => 'hidden',
      '#value' => $file,
    );
  }

  $form['per_folder_js_checking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Per Folder JS Checking'),
    '#description' => t('Can check JS files, even if not used inside of AdvAgg.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $files = advagg_validator_scan_all_dirs('js');
  sort($files);
  foreach ($files as $file) {
    $dir = dirname($file);
    if (!isset($form['per_folder_js_checking'][$dir])) {
      $form['per_folder_js_checking'][$dir] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($dir),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form['per_folder_js_checking'][$dir]['button_' . $dir] = array(
        '#type' => 'button',
        '#value' => t('Check this Directory: @dirname', array('@dirname' => $dir)),
        '#attributes' => array(
          'class' => array('advagg_validator_js'),
        ),
      );
      $form['per_folder_js_checking'][$dir]["wrapper"] = array(
        "#markup" => "<div class='results'></div>",
      );
    }
    $form_api_filename = str_replace(array('/', '.'), array('__', '--'), $file);
    $form['per_folder_js_checking'][$dir][$form_api_filename] = array(
      '#markup' => check_plain($file) . ' ',
    );
    $form['per_folder_js_checking'][$dir]['hidden_' . $form_api_filename] = array(
      '#type' => 'hidden',
      '#value' => $file,
      '#attributes' => array(
        'class' => array('filenames'),
      ),
    );
  }
  $form['#attached']['js'][] = array(
    'data' => '//rawgithub.com/jshint/jshint/master/dist/jshint.js',
    'type' => 'external',
  );
  $form['#attached']['js'][] = array(
    'data' => drupal_get_path('module', 'advagg_validator') . '/advagg_validator.js',
    'type' => 'file',
  );
  $settings = array(
    'browser' => TRUE,
    'curly' => TRUE,
    'eqeqeq' => TRUE,
    'forin' => TRUE,
    'latedef' => TRUE,
    'newcap' => TRUE,
    'noarg' => TRUE,
    'strict' => TRUE,
    'trailing' => TRUE,
    'undef' => TRUE,
    'unused' => TRUE,
    'predef' => array(
      'Drupal' => FALSE,
      'drupalSettings' => FALSE,
      'domready' => FALSE,
      'jQuery' => FALSE,
      '_' => FALSE,
      'matchMedia' => FALSE,
      'Backbone' => FALSE,
      'Modernizr' => FALSE,
      'VIE' => FALSE,
      'CKEDITOR' => FALSE,
    ),
  );
  $form['#attached']['js'][] = array(
    'data' => array('jshint' => $settings),
    'type' => 'setting',
  );

  return $form;
}

// Submit callbacks.
/**
 * Display file info in a drupal message.
 */
function advagg_validator_admin_test_advagg_css_submit($form, &$form_state) {
  module_load_include('inc', 'advagg_validator', 'advagg_validator');

  if (!empty($form_state['input']['ajax_page_state'])) {
    return;
  }
  $info = advagg_validator_test_advagg_css();
  $info = advagg_validator_admin_hide_good_files($info);
  if (module_exists('httprl')) {
    $output = httprl_pr($info);
  }
  else {
    $output = '<pre>' . check_plain(print_r($info, TRUE)) . '</pre>';
  }
  drupal_set_message($output);
}

/**
 * Display file info via ajax callback.
 */
function advagg_validator_admin_test_advagg_css_callback($form, &$form_state) {
  module_load_include('inc', 'advagg_validator', 'advagg_validator');

  if (!empty($form_state['values']['error'])) {
    return '<div id="advagg-file-info-ajax"></div>';
  }
  $info = advagg_validator_test_advagg_css();
  if (empty($info)) {
    return '<div id="advagg-file-info-ajax"></div>';
  }
  $info = advagg_validator_admin_hide_good_files($info);
  if (module_exists('httprl')) {
    $output = httprl_pr($info);
  }
  else {
    $output = '<pre>' . print_r($info, TRUE) . '</pre>';
  }
  return '<div id="advagg-validator-css-validator-ajax">' . $output . '</div>';
}

/**
 * Display file info in a drupal message.
 */
function advagg_validator_admin_test_advagg_css_directory_submit($form, &$form_state) {
  if (!empty($form_state['input']['ajax_page_state']) || empty($form_state['values']['op']) || strpos($form_state['values']['op'], t('Check this Directory:')) !== 0) {
    return;
  }
  module_load_include('inc', 'advagg_validator', 'advagg_validator');

  $dir = trim(substr($form_state['values']['op'], strpos($form_state['values']['op'], ':')+1));
  $slash_count = substr_count('/' . $dir, '/');

  // Get list of files.
  $files = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'hidden') === FALSE || strpos($value, $dir) === FALSE || substr_count($value, '/') > $slash_count || ($dir == '.' && substr_count($value, '/') > 0)) {
      continue;
    }
    $files[] = $value;
  }

  // Check list.
  $info = advagg_validator_test_css_files($files);
  $info = advagg_validator_admin_hide_good_files($info);
  if (module_exists('httprl')) {
    $output = httprl_pr($info);
  }
  else {
    $output = '<pre>' . check_plain(print_r($info, TRUE)) . '</pre>';
  }
  drupal_set_message($output);

}

/**
 * Display file info via ajax callback.
 */
function advagg_validator_admin_test_advagg_css_directory_callback($form, &$form_state) {
  if (empty($form_state['values']['op']) || strpos($form_state['values']['op'], t('Check this Directory:')) !== 0) {
    return '<div></div>';
  }
  module_load_include('inc', 'advagg_validator', 'advagg_validator');

  $dir = trim(substr($form_state['values']['op'], strpos($form_state['values']['op'], ':')+1));
  $slash_count = substr_count('/' . $dir, '/');

  // Get list of files.
  $files = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'hidden') === FALSE || strpos($value, $dir) === FALSE || substr_count($value, '/') > $slash_count || ($dir == '.' && substr_count($value, '/') > 0)) {
      continue;
    }
    $files[] = $value;
  }

  // Check list.
  $info = advagg_validator_test_css_files($files);
  if (empty($info)) {
    return '<div id="' . $wrapper . '"></div>';
  }
  $info = advagg_validator_admin_hide_good_files($info);
  if (module_exists('httprl')) {
    $output = httprl_pr($info);
  }
  else {
    $output = '<pre>' . print_r($info, TRUE) . '</pre>';
  }

  $form_api_dirname = str_replace(array('/', '.'), array('__', '--'), $dir);
  $wrapper = 'advagg-validator-css-validator-ajax' . $form_api_dirname;
  return '<div id="' . $wrapper . '">' . $output . '</div>';
}

/**
 * Display file info in a drupal message.
 */
function advagg_validator_admin_test_all_css_submit($form, &$form_state) {
  module_load_include('inc', 'advagg_validator', 'advagg_validator');

  if (!empty($form_state['input']['ajax_page_state'])) {
    return;
  }
  $info = advagg_validator_test_all_css();
  $info = advagg_validator_admin_hide_good_files($info);
  if (module_exists('httprl')) {
    $output = httprl_pr($info);
  }
  else {
    $output = '<pre>' . check_plain(print_r($info, TRUE)) . '</pre>';
  }
  drupal_set_message($output);
}

/**
 * Display file info via ajax callback.
 */
function advagg_validator_admin_test_all_css_callback($form, &$form_state) {
  module_load_include('inc', 'advagg_validator', 'advagg_validator');

  if (!empty($form_state['values']['error'])) {
    return '<div id="advagg-file-info-ajax"></div>';
  }
  $info = advagg_validator_test_all_css();
  if (empty($info)) {
    return '<div id="advagg-file-info-ajax"></div>';
  }
  $info = advagg_validator_admin_hide_good_files($info);
  if (module_exists('httprl')) {
    $output = httprl_pr($info);
  }
  else {
    $output = '<pre>' . print_r($info, TRUE) . '</pre>';
  }
  return '<div id="advagg-validator-css-validator-ajax">' . $output . '</div>';
}

/**
 * Do not display info on a file if it is valid.
 *
 * @param array $info
 *   Array containing info about validators and if the file is valid.
 * @return array
 *   $info array minus data if that file is valid.
 */
function advagg_validator_admin_hide_good_files($info) {
  $output = array();
  foreach ($info as $filename => $validators) {
    foreach ($validators as $v_name => $v_data) {
      if (!empty($v_data['validity'])) {
        $output[$filename][$v_name] = 'valid';
        continue;
      }
      $output[$filename][$v_name] = $v_data;
    }
  }
  return $output;
}
