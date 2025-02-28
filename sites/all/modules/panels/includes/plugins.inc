<?php

/**
 * @file
 *
 * Contains helper code for plugins and contexts.
 */

/**
 * Determine if a pane is visible.
 *
 * @param $pane
 *   The pane object to test for access.
 * @param $display
 *   The display object containing the pane object to be tested.
 */
function panels_pane_access($pane, $display) {
  ctools_include('context');
  return ctools_access($pane->access, $display->context);
}

/**
 * Get a list of panels available in the layout.
 */
function panels_get_regions($layout, $display) {
  if ($function = ctools_plugin_get_function($layout, 'regions function')) {
    return $function($display, $display->layout_settings, $layout);
  }

  if (!empty($layout['regions'])) {
    return $layout['regions'];
  }

  return array();
}

/**
 * Get cached content for a given display and possibly pane.
 *
 * @return
 *   The cached content, or FALSE to indicate no cached content exists.
 */
function panels_get_cached_content($display, $args, $context, $pane = NULL) {
  // Never use cache on a POST
  if (!empty($_POST)) {
    return FALSE;
  }

  $method = $pane ? $pane->cache['method'] : $display->cache['method'];
  $function = panels_plugin_get_function('cache', $method, 'cache get');

  if (!$function) {
    return FALSE;
  }

  $conf = $pane ? $pane->cache['settings'] : $display->cache['settings'];
  $cache = $function($conf, $display, $args, $context, $pane);
  if (empty($cache)) {
    return FALSE;
  }

  // restore it.
  $cache->restore();
  return $cache;
}

/**
 * Store cached content for a given display and possibly pane.
 */
function panels_set_cached_content($cache, $display, $args, $context, $pane = NULL) {
  // Never use cache on a POST
  if (!empty($_POST)) {
    return FALSE;
  }

  $method = $pane ? $pane->cache['method'] : $display->cache['method'];
  $function = panels_plugin_get_function('cache', $method, 'cache set');

  if (!$function) {
    return FALSE;
  }

  $conf = $pane ? $pane->cache['settings'] : $display->cache['settings'];

  // snapshot it.
  $cache->cache();
  return $function($conf, $cache, $display, $args, $context, $pane);
}

/**
 * Clear all cached content for a display.
 */
function panels_clear_cached_content($display) {
  // Figure out every method we might be using to cache content in this display:
  $methods = array();
  if (!empty($display->cache['method'])) {
    $methods[$display->cache['method']] = TRUE;
  }

  foreach ($display->content as $pane) {
    if (!empty($pane->cache['method'])) {
      $methods[$pane->cache['method']] = TRUE;
    }
  }

  foreach (array_keys($methods) as $method) {
    $function = panels_plugin_get_function('cache', $method, 'cache clear');
    if ($function) {
      $function($display);
    }
  }
}

/**
 * An object to hold caching information while it is happening.
 */
class panels_cache_object {
  var $content = '';
  var $head = NULL;
  var $css = NULL;
  var $js = NULL;
  var $tokens = NULL;
  var $ready = FALSE;

  /**
   * When constructed, take a snapshot of our existing out of band data.
   */
  function panels_cache_object() {
    $this->head = drupal_add_html_head();
    $this->css = drupal_add_css();
    $this->tokens = ctools_set_page_token();
    $this->js = drupal_add_js();
  }

  /**
   * Add content to the cache. This assumes a pure stream;
   * use set_content() if it's something else.
   */
  function add_content($content) {
    $this->content .= $content;
  }

  function set_content($content) {
    $this->content = $content;
  }

  /**
   * Set the object for storing. This overwrites.
   */
  function cache() {
    if ($this->ready) {
      return;
    }

    $this->ready = TRUE;

    // Simple replacement for head
    $this->head = str_replace($this->head, '', drupal_add_html_head());

    // Slightly less simple for CSS:
    $css = drupal_add_css();
    $start = $this->css;
    $this->css = array();

    foreach ($css as $name => $data) {
      if (!isset($start[$name])) {
        $this->css[$name] = $data;
      }
    }

    $js = drupal_add_js();

    $start = $this->js;
    $this->js = array();

    // If there are any differences between the old and the new javascript then
    // store them to be added later.
//ADDED from https://drupal.org/files/panels-array_diff_assoc-php5.4-issue-1762290-4-extended.patch in regards to https://drupal.org/node/1762290
//    if ($diff = array_diff_assoc($js, $start)) {
    if ($diff = drupal_array_diff_assoc_recursive($js, $start)) {
//END
      $this->js = $diff;
    }
    // Special case the settings key and get the difference of the data.
//ADDED from https://drupal.org/files/panels-array_diff_assoc-php5.4-issue-1762290-4-extended.patch in regards to https://drupal.org/node/1762290
//    if ($settings_diff = array_diff_assoc($js['settings']['data'], $start['settings']['data'])) {
    if ($settings_diff = drupal_array_diff_assoc_recursive($js['settings']['data'], $start['settings']['data'])) {
//END
      $this->js['settings'] = $settings_diff;
    }

    // And for tokens:
    $tokens = ctools_set_page_token();
    foreach ($this->tokens as $token => $argument) {
      if (isset($tokens[$token])) {
        unset($tokens[$token]);
      }
    }

    $this->tokens = $tokens;
  }

  /**
   * Restore out of band data saved to cache.
   */
  function restore() {
    if (!empty($this->head)) {
      drupal_add_html_head($this->head);
    }
    if (!empty($this->css)) {
      foreach ($this->css as $args) {
        drupal_add_css($args['data'], $args);
      }
    }
    if (!empty($this->js)) {
      foreach ($this->js as $key => $args) {
        if ($key !== 'settings') {
          drupal_add_js($args['data'], $args);
        }
        else {
          foreach ($args as $setting) {
            drupal_add_js($setting, 'setting');
          }
        }
      }
    }

    if (!empty($this->tokens)) {
      foreach ($this->tokens as $token => $key) {
        list($type, $argument) = $key;
        ctools_set_page_token($token, $type, $argument);
      }
    }
  }
}

/**
 * Get the title of a pane.
 *
 * @deprecated @todo this function should be removed.
 *
 * @param $pane
 *   The $pane object.
 */
function panels_get_pane_title(&$pane, $context = array(), $incoming_content = NULL) {
  ctools_include('content');
  return ctools_content_admin_title($pane->type, $pane->subtype, $pane->configuration, $context);
}

/**
 * Fetch metadata on a specific layout plugin.
 *
 * @param $layout
 *   Name of a panel layout. If the layout name contains a ':' this
 *   indicates that we need to separate the sublayout out and
 *   load it individually.
 *
 * @return
 *   An array with information about the requested panel layout.
 */
function panels_get_layout($layout) {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'layouts', $layout);
}

/**
 * Fetch metadata for all layout plugins.
 *
 * @return
 *   An array of arrays with information about all available panel layouts.
 */
function panels_get_layouts() {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'layouts');
}

/**
 * Fetch metadata for all layout plugins that provide builders.
 *
 * The layout builders allow reusable layouts be stored in the database and
 * exported. Since there are different methods, we are not limiting this
 * to just one plugin.
 *
 * @return
 *   An array of arrays with information about panel layouts with builders.
 */
function panels_get_layout_builders() {
  ctools_include('plugins');
  $plugins = ctools_get_plugins('panels', 'layouts');
  $builders = array();
  foreach ($plugins as $name => $plugin) {
    if (!empty($plugin['builder'])) {
      $builders[$name] = $plugin;
    }
  }

  return $builders;
}

/**
 * Fetch metadata on a specific style plugin.
 *
 * @param $style
 *   Name of a panel style.
 *
 * @return
 *   An array with information about the requested panel style.
 */
function panels_get_style($style) {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'styles', $style);
}

/**
 * Fetch metadata for all style plugins.
 *
 * @return
 *   An array of arrays with information about all available panel styles.
 */
function panels_get_styles() {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'styles');
}

/**
 * Fetch metadata on a specific caching plugin.
 *
 * @param $cache
 *   Name of a panel cache.
 *
 * @return
 *   An array with information about the requested panel cache.
 */
function panels_get_cache($cache) {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'cache', $cache);
}

/**
 * Fetch metadata for all context plugins.
 *
 * @return
 *   An array of arrays with information about all available panel caches.
 */
function panels_get_caches() {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'cache');
}

/**
 * Fetch metadata on a specific display renderer plugin.
 *
 * @return
 *   An array of arrays with information about the requested panels display
 *   renderer.
 */
function panels_get_display_renderer($renderer) {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'display_renderers', $renderer);
}

/**
 * Fetch metadata for all display renderer plugins.
 *
 * @return
 *   An array of arrays with information about all available panels display
 *   renderer.
 */
function panels_get_display_renderers() {
  ctools_include('plugins');
  return ctools_get_plugins('panels', 'display_renderers');
}

/**
 * Get and initialize the class to handle rendering a display.
 *
 * @return
 *   Either the instantiated renderer or FALSE if one could not be found.
 */
function panels_get_renderer_handler($plugin, &$display) {
  if (is_string($plugin)) {
    $plugin = panels_get_display_renderer($plugin);
  }

  $class = ctools_plugin_get_class($plugin, 'renderer');
  if ($class) {
    $renderer = new $class();
    $renderer->init($plugin, $display);
    return $renderer;
  }

  return FALSE;
}

/**
 * Choose a renderer for a display based on a render pipeline setting.
 */
function panels_get_renderer($pipeline_name, &$display) {
  // Load the pipeline
  ctools_include('export');
  $pipeline = ctools_export_crud_load('panels_renderer_pipeline', $pipeline_name);

  // If we can't, or it has no renderers, default.
  if (!$pipeline || empty($pipeline->settings['renderers'])) {
    return panels_get_renderer_handler('standard', $display);
  }

  // Get contexts set on the pipeline:
  $contexts = array();
  if (!empty($pipeline->settings['contexts'])) {
    $contexts = ctools_context_load_contexts($pipeline->settings['context']);
  }

  // Cycle through our renderers and see.
  foreach ($pipeline->settings['renderers'] as $candidate) {
    // See if this passes selection criteria.
    if (!ctools_access($candidate['access'], $contexts)) {
      continue;
    }

    $renderer = panels_get_renderer_handler($candidate['renderer'], $display);

    if (!empty($candidate['options'])) {
      $renderer->set_options($candidate['options']);
    }

    return $renderer;
  }

  // Fall through. If no renderer is selected, use the standard renderer
  return panels_get_renderer_handler('standard', $display);
}

/**
 * Sort callback for sorting renderer pipelines.
 *
 * Sort first by weight, then by title.
 */
function _panels_renderer_pipeline_sort($a, $b) {
  if ($a->weight == $b->weight) {
      if ($a->admin_title == $b->admin_title) {
        return 0;
      }
    return ($a->admin_title < $b->admin_title) ? -1 : 1;
  }
  return ($a->weight < $b->weight) ? -1 : 1;
}

/**
 * Get a list of available renderer pipelines.
 *
 * This can be used to form a select or radios widget by enabling
 * sorting. Descriptions are left in.
 */
function panels_get_renderer_pipelines($sort = TRUE) {
  ctools_include('export');
  $pipelines = ctools_export_crud_load_all('panels_renderer_pipeline');
  if ($sort) {
    uasort($pipelines, '_panels_renderer_pipeline_sort');
  }

  return $pipelines;
}

/**
 * Get a function from a plugin, if it exists.
 *
 * @param $plugin
 *   The type of plugin
 * @param $which
 *   Either the loaded plugin object (or the same data in array form)
 *   or a string with the name of the desired the specific plugin.
 * @param $function_name
 *   The identifier of the function. For example, 'settings form'.
 *
 * @return
 *   The actual name of the function to call, or NULL if the function
 *   does not exist.
 *
 * @deprecated All calls to this function should be removed.
 */
function panels_plugin_get_function($plugin, $which, $function_name) {
  ctools_include('plugins');
  if (is_object($which) || is_array($which)) {
    return ctools_plugin_get_function($which, $function_name);
  }
  else {
    return ctools_plugin_load_function('panels', $plugin, $which, $function_name);
  }
}
//ADDED from https://drupal.org/files/panels-array_diff_assoc-php5.4-issue-1762290-4-extended.patch in regards to https://drupal.org/node/1762290
if (!function_exists('drupal_array_diff_assoc_recursive')) {
  /**
   * Drop-in replacement for a function added in Drupal 7.23.
   *
   * This is an exact copy of the function added in Drupal 7.23 to avoid issues
   * with array_diff_assoc and php 5.4.
   *
   * @link https://drupal.org/node/1850798 Related Drupal issue
   * @link https://drupal.org/node/1762290 Related Panels issue
   */
  function drupal_array_diff_assoc_recursive($array1, $array2) {
    $difference = array();

    foreach ($array1 as $key => $value) {
      if (is_array($value)) {
        if (!array_key_exists($key, $array2) || !is_array($array2[$key])) {
          $difference[$key] = $value;
        }
        else {
          $new_diff = drupal_array_diff_assoc_recursive($value, $array2[$key]);
          if (!empty($new_diff)) {
            $difference[$key] = $new_diff;
          }
        }
      }
      elseif (!array_key_exists($key, $array2) || $array2[$key] !== $value) {
        $difference[$key] = $value;
      }
    }

    return $difference;
  }
}
//END