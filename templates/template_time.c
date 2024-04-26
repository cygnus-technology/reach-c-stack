#ifdef INCLUDE_TIME_SERVICE
int crcb_time_get(cr_TimeGetResponse *response)
{
  response->seconds_utc = (rsl_get_system_uptime() + time_offset) / 1000;
  response->has_timezone = sCr_param_val[PARAM_TIMEZONE_ENABLED].value.bool_value;
  if (!response->has_timezone)
  {
    response->seconds_utc += sCr_param_val[PARAM_TIMEZONE_OFFSET].value.int32_value;
  }
  else
  {
    response->timezone = sCr_param_val[PARAM_TIMEZONE_OFFSET].value.int32_value;
  }
  return 0;
}

int crcb_time_set(const cr_TimeSetRequest *request)
{
  time_offset = (request->seconds_utc * 1000) - rsl_get_system_uptime();
  if (request->has_timezone)
  {
    cr_ParameterValue param;
    param.parameter_id = PARAM_TIMEZONE_OFFSET;
    param.which_value = cr_ParameterValue_int32_value_tag;
    param.timestamp = (uint32_t) rsl_get_system_uptime();
    param.value.int32_value = request->timezone;
    crcb_parameter_write(PARAM_TIMEZONE_OFFSET, &param);
  }
  return 0;
}
#endif

