<?php
/**
 * @file
 * Date module integration as a date source for TimelineJS.
 */

$plugin = array(
  'name' => t('Date field for TimelineJS'),
  'callback' => 'views_timelinejs_date_source_date_field_conversion',
  'handler_name' => 'views_handler_field_field',
  'field_type' => 'datetime',
);

/**
 * Converts date field formats, forces TimelineJS to ignore browser timezone.
 *
 * Converts different expected date formats to one global date format to be used
 * in TimelineJS. Uses Drupal's TZ, not field TZ. Adds Z to ISODate to force
 * TimelineJS to ignore browser timezone.
 *
 * @return string
 *   Formatted date.
 */
function views_timelinejs_date_source_date_field_conversion($date, $output_format, $options = array()) {
  $granularity = isset($options['field']['granularity']) ? $options['field']['granularity'] : '';
  $input_format = $options['field']['date_format'];
  $tz_handling = $options['field']['tz_handling'];
  $timezone = date_get_timezone($tz_handling);
  $db_timezone = date_get_timezone_db($tz_handling);
  switch ($output_format) {
    case 'csv':
      $ret['value'] = views_timelinejs_convert_to_csv($date['value'], $input_format, $timezone, $db_timezone, $granularity);
      if (isset($date['value2'])) {
        $ret['value2'] = views_timelinejs_convert_to_csv($date['value2'], $input_format, $timezone, $db_timezone, $granularity);
      }
      break;

    case 'timestamp':
      $ret['value'] = views_timelinejs_convert_to_timestamp($date['value'], $input_format, $timezone, $db_timezone, $granularity);
      if (isset($date['value2'])) {
        $ret['value2'] = views_timelinejs_convert_to_timestamp($date['value2'], $input_format, $timezone, $db_timezone, $granularity);
      }
      break;

  }

  // If times are the same, don't duplicate.
  if (isset($ret['value2']) && $ret['value'] == $ret['value2']) {
    unset($ret['value2']);
  }

  return $ret;
}
