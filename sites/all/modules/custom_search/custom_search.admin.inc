<?php

/**
 * @file
 * Admin settings for custom search
 */

/**
 * General settings.
 */
function custom_search_admin() {
  // Basic settings.
  $form = _custom_search_default_admin_form();
  // Custom search paths.
  $form = array_merge($form, _custom_search_custom_paths_admin_form());
  // Ordering.
  $form = array_merge($form, _custom_search_ordering_admin_form());

  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  $form['#submit'][] = 'custom_search_admin_submit';
  return system_settings_form($form);
}

/**
 * Submit callback.
 */
function custom_search_admin_submit($form, &$form_state) {
  // Saves the ordering.
  foreach ($form_state['values']['custom_search_order'] as $key => $data) {
    variable_set('custom_search_' . $key . '_weight', $data['sort']);
    variable_set('custom_search_' . $key . '_region', $data['region']);
  }

  // Uploads an image.
  $directory_path = 'public://custom_search';
  file_prepare_directory($directory_path, FILE_CREATE_DIRECTORY);
  // Check for a new uploaded image.
  if ($file = file_save_upload('custom_search_image', array('file_validate_is_image' => array()))) {
    if ($filepath = file_unmanaged_copy($file->uri, $directory_path)) {
      $form_state['values']['custom_search_image_path'] = $filepath;
    }
  }
}

/**
 * Content settings.
 */
function custom_search_content_admin() {
  return system_settings_form(_custom_search_content_admin_form());
}

/**
 * Results page settings.
 */
function custom_search_results_admin() {
  $form['custom_search_target'] = array(
    '#type'     => 'select',
    '#title'    => t('Results page opens in'),
    '#options'  => array(
      '_self'   => t('the same window'),
      '_blank'  => t('a new window'),
    ),
    '#default_value'  => variable_get('custom_search_target', '_self'),
  );

  $form['search_form'] = array(
    '#type'   => 'fieldset',
    '#title'  => t('Search form'),
  );
  $form['search_form']['custom_search_results_search'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Display basic search'),
    '#default_value'  => variable_get('custom_search_results_search', TRUE),
  );
  $form['search_form']['custom_search_results_advanced_search'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Display advanced search'),
    '#default_value'  => variable_get('custom_search_results_advanced_search', TRUE),
  );
  $form['search_form']['advanced'] = array(
    '#type'         => 'fieldset',
    '#title'        => t('Advanced search'),
    '#states' => array(
      'visible' => array(
        ':input[name="custom_search_results_advanced_search"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['search_form']['advanced']['custom_search_results_advanced_search_collapsible'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Collapsible'),
    '#default_value'  => variable_get('custom_search_results_advanced_search_collapsible', TRUE),
  );
  $form['search_form']['advanced']['custom_search_results_advanced_search_collapsed'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Collapsed'),
    '#default_value'  => variable_get('custom_search_results_advanced_search_collapsed', TRUE),
  );
  $form['search_form']['advanced']['criteria'] = array(
    '#type'         => 'fieldset',
    '#title'        => t('Criteria'),
    '#description'  => t('Select the criteria to display on the advanced search form.'),
    '#collapsible'  => TRUE,
    '#collapsed'    => TRUE,
  );
  $form['search_form']['advanced']['criteria']['custom_search_advanced_or_display'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Or'),
    '#default_value'  => variable_get('custom_search_advanced_or_display', TRUE),
  );
//ADDED for AND search
  $form['search_form']['advanced']['criteria']['custom_search_advanced_and_display'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('And'),
    '#default_value'  => variable_get('custom_search_advanced_and_display', TRUE),
  );
//END
  $form['search_form']['advanced']['criteria']['custom_search_advanced_phrase_display'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Phrase'),
    '#default_value'  => variable_get('custom_search_advanced_phrase_display', TRUE),
  );
  $form['search_form']['advanced']['criteria']['custom_search_advanced_negative_display'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Negative'),
    '#default_value'  => variable_get('custom_search_advanced_negative_display', TRUE),
  );
  $form['search_form']['advanced']['types'] = array(
    '#type'         => 'fieldset',
    '#title'        => t('Content types'),
    '#description'  => t('Select the content types to display on the advanced search form.'),
    '#collapsible'  => TRUE,
    '#collapsed'    => TRUE,
  );
  $options = node_type_get_names();
  foreach ($options as $type => $name) {
    $form['search_form']['advanced']['types']['custom_search_advanced_type_' . $type . '_display'] = array(
      '#type'           => 'checkbox',
      '#title'          => $name,
      '#default_value'  => variable_get('custom_search_advanced_type_' . $type . '_display', TRUE),
    );
  }

  // Languages.
  $language_options = array();
  foreach (language_list('language') as $key => $entity) {
    if ($entity->enabled) {
      $language_options[$key] = $entity->name;
    }
  }
  if (count($language_options) > 1) {
    $form['search_form']['advanced']['languages'] = array(
      '#type'         => 'fieldset',
      '#title'        => t('Languages'),
      '#description'  => t('Select the languages to display on the advanced search form.'),
      '#collapsible'  => TRUE,
      '#collapsed'    => TRUE,
    );
    foreach ($language_options as $key => $name) {
      $form['search_form']['advanced']['languages']['custom_search_advanced_language_' . $key . '_display'] = array(
        '#type'           => 'checkbox',
        '#title'          => $name,
        '#default_value'  => variable_get('custom_search_advanced_language_' . $key . '_display', TRUE),
      );
    }
  }

  $form['results'] = array(
    '#type'         => 'fieldset',
    '#title'        => t('Results'),
    '#description'  => t('Select data to display below each result.'),
  );
  $form['results']['custom_search_results_info_type'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Content type'),
    '#default_value'  => variable_get('custom_search_results_info_type', TRUE),
  );
  $form['results']['custom_search_results_info_user'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('User'),
    '#default_value'  => variable_get('custom_search_results_info_user', TRUE),
  );
  $form['results']['custom_search_results_info_date'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Date'),
    '#default_value'  => variable_get('custom_search_results_info_date', TRUE),
  );
  if (module_exists('comment')) {
    $form['results']['custom_search_results_info_comment'] = array(
      '#type'           => 'checkbox',
      '#title'          => t('Comments'),
      '#default_value'  => variable_get('custom_search_results_info_comment', TRUE),
    );
  }

  $form['filter'] = array(
    '#type'         => 'fieldset',
    '#title'        => t('Filter'),
    '#description'  => t('Add links to filter the results by content type.'),
  );
  $form['filter']['custom_search_filter'] = array(
    '#type'           => 'select',
    '#title'          => t('Position'),
    '#options'        => array(
      'disabled'  => t('Disabled'),
      'above'     => t('Above results'),
      'below'     => t('Below results'),
    ),
    '#default_value'  => variable_get('custom_search_filter', 'disabled'),
  );
  $form['filter']['custom_search_filter_label'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Label text'),
    '#default_value'  => variable_get('custom_search_filter_label', CUSTOM_SEARCH_FILTER_LABEL_DEFAULT),
    '#description'    => t('Enter the label text for the list. The default value is "!default".', array('!default' => CUSTOM_SEARCH_FILTER_LABEL_DEFAULT)),
    '#states' => array(
      'invisible' => array(
        ':input[name="custom_search_filter"]' => array('value' => 'disabled'),
      ),
    ),
  );

  return system_settings_form($form);
}
