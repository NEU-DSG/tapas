<?php

/**
 * @file
 * Theme functions for the Facet API module.
 */

/**
 * Returns HTML for a "missing" facet link.
 *
 * @param $variables
 *   An associative array containing:
 *   - field_name: The name of the facet field.
 *
 * @ingroup themeable
 */
function theme_facetapi_facet_missing($variables) {
  return t('Missing %field_name', array('%field_name' => $variables['field_name']));
}

/**
 * Returns HTML for the facet title, usually the title of the block.
 *
 * @param $variables
 *   An associative array containing:
 *   - title: The title of the facet.
 *   - facet: The facet definition as returned by facetapi_facet_load().
 *
 * @ingroup themeable
 */
function theme_facetapi_title($variables) {
	//ADDED to change display title dont know why they make it lowercase
	//  return t('Filter by @title:', array('@title' => drupal_strtolower($variables['title'])));
  //return t('Filter by '.$variables["title"].':');
  //END
}

/**
 * Returns HTML for an inactive facet item.
 *
 * @param $variables
 *   An associative array containing the keys 'text', 'path', 'options', and
 *   'count'. See the l() and theme_facetapi_count() functions for information
 *   about these variables.
 *
 * @ingroup themeable
 */
function theme_facetapi_link_inactive($variables) {
  // Builds accessible markup.
  // @see http://drupal.org/node/1316580
  $accessible_vars = array(
    'text' => $variables['text'],
    'active' => FALSE,
  );
  $accessible_markup = theme('facetapi_accessible_markup', $accessible_vars);

  // Sanitizes the link text if necessary.
  $sanitize = empty($variables['options']['html']);
  $variables['text'] = ($sanitize) ? check_plain($variables['text']) : $variables['text'];

  // Adds count to link if one was passed.
  if (isset($variables['count'])) {
    $variables['text'] .= ' ' . theme('facetapi_count', $variables);
  }

  // Resets link text, sets to options to HTML since we already sanitized the
  // link text and are providing additional markup for accessibility.
  $variables['text'] .= $accessible_markup;
  $variables['options']['html'] = TRUE;
  return theme_link($variables);
}

/**
 * Returns HTML for the active facet item's count.
 *
 * @param $variables
 *   An associative array containing:
 *   - count: The item's facet count.
 *
 * @ingroup themeable
 */
function theme_facetapi_count($variables) {
  return '(' . (int) $variables['count'] . ')';
}

/**
 * Returns HTML for an inactive facet item.
 *
 * @param $variables
 *   An associative array containing the keys 'text', 'path', and 'options'. See
 *   the l() function for information about these variables.
 *
 * @see l()
 *
 * @ingroup themeable
 */
function theme_facetapi_link_active($variables) {

  // Sanitizes the link text if necessary.
  $sanitize = empty($variables['options']['html']);
  $link_text = ($sanitize) ? check_plain($variables['text']) : $variables['text'];

  // Theme function variables fro accessible markup.
  // @see http://drupal.org/node/1316580
  $accessible_vars = array(
    'text' => $variables['text'],
    'active' => TRUE,
  );

  // Builds link, passes through t() which gives us the ability to change the
  // position of the widget on a per-language basis.
  $replacements = array(
    '!facetapi_deactivate_widget' => theme('facetapi_deactivate_widget', $variables),
    '!facetapi_accessible_markup' => theme('facetapi_accessible_markup', $accessible_vars),
  );
  $variables['text'] = t('!facetapi_deactivate_widget !facetapi_accessible_markup', $replacements);
  $variables['options']['html'] = TRUE;
  return theme_link($variables) . $link_text;
}

/**
 * Returns HTML for the deactivation widget.
 *
 * @param $variables
 *   An associative array containing the keys 'text', 'path', and 'options'. See
 *   the l() function for information about these variables.
 *
 * @see l()
 * @see theme_facetapi_link_active()
 *
 * @ingroup themable
 */
function theme_facetapi_deactivate_widget($variables) {
  return '(-)';
}

/**
 * Returns HTML that adds accessible markup to facet links.
 *
 * @param $variables
 *   An associative array containing:
 *   - text: The text of the facet link.
 *   - active: Whether the item is active or not.
 *
 * @ingroup themeable
 *
 * @see http://drupal.org/node/1316580
 */
function theme_facetapi_accessible_markup($variables) {
  $vars = array('@text' => $variables['text']);
  $text = ($variables['active']) ? t('Remove @text filter', $vars) : t('Apply @text filter', $vars);
  // Add spaces before and after the text, since other content may be displayed
  // inline with this and we don't want the words to run together. However, the
  // spaces must be inside the <span> so as not to disrupt the layout for
  // sighted users.
  return '<span class="element-invisible"> ' . $text . ' </span>';
}
