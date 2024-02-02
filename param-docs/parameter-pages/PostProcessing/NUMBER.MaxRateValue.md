# Parameter `<NUMBER>.MaxRateValue`
Default Value: `0,05`

Maximum allowed change between two readings, if exceeded the last reading will be rejected. Depending on the settings of `<NUMBER>.MaxRateType` the `MaxRateValue` is either treated as the difference between the two measurements (`AbsoluteChange` = difference) not taking the set time interval into account or as the difference normalized to the interval (`RateChange` = difference per minute).

If negative rate is disallowed and no maximum rate value is set, one false high reading will lead to a period of missing measurements until the measurement reaches the previous false high reading. E.g. if the counter is at `600,00` and it's read incorrectly as` 610,00`, all measurements will be skipped until the counter reaches `610,00`. Setting the MaxRateValue to `0,05` leads to a rejection of all readings with a difference `> 0,05`, in this case `610,00`. The rejection also applies to correct readings with a difference `> 0,05`!
