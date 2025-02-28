<?php

/**
 * @file
 * Path generation for Apache Solr Search.
 *
 * Available vars:
 * $keywords: user input
 * $types: content types (machine names[])
 * $terms: taxonomy terms (tids[])
 * $keys: complete search phrase, as core would have done it
 *
 * To return:
 * the complete search path
 */

function _custom_search_apachesolr_search($variables) {
//ADDED from older version (7.x-1.11+13-dev) of custom search
/*
  // Use the search info for the apachesolr module to get the search path.
  $solr_info = apachesolr_search_search_info();
  $type = 'search/' . $solr_info['path'] . '/' . $variables['keywords'];
*/
  // Set default path in case the core search page path isn't set yet.
  $path = 'search/apachesolr_search';
  $solr_info = apachesolr_search_page_load('core_search');
  if (isset($solr_info['search_path'])) {
    // Use the configured Solr search path instead of default path.
    $path = $solr_info['search_path'];
  }
  $type = $path . '/' . $variables['keywords'];
//END
  $keys = array();

  if (count($variables['types']) && !in_array('all', $variables['types'])) {
    foreach ($variables['types'] as $t) {
      $keys['f[' . count($keys) . ']'] = 'bundle:' . $t;
    }
  }

  if (module_exists('taxonomy') && count($variables['terms'])) {
    // Get all fields info to get correct filter names.
    $fields = field_info_fields();
    $taxonomy_fields = array();
    foreach ($fields as $name => $settings) {
      if ($settings['type'] == 'taxonomy_term_reference') {
        $voc = taxonomy_vocabulary_machine_name_load($settings['settings']['allowed_values'][0]['vocabulary']);
        $taxonomy_fields[$voc->vid] = $name;
      }
    }
    // Build keys for taxonomy.
    foreach ($variables['terms'] as $t) {
      $vocid = taxonomy_term_load($t)->vid;
      $keys['f[' . count($keys) . ']'] = 'im_' . $taxonomy_fields[$vocid] . ':' . $t;
    }
  }
  
//ADDED to preserve facets selected
$the_url = request_uri();
  $url=parse_url($the_url, PHP_URL_QUERY);
 parse_str($url, $keys);
//END
  return array('path' => $type, 'query' => $keys);
}
