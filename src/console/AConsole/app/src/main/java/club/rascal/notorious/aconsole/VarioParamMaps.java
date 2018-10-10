package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-26.
 */

public class VarioParamMaps {

    public static final VarioParamMapData[] mMaps;

    static {
        mMaps = new VarioParamMapData[] {

        // GliderInfo
        new VarioParamMapData(VarioParam.GLIDER_TYPE, R.string.pref_key_glider_type, VarioParamMapData.TYPE_INTEGER, "1"),
        new VarioParamMapData(VarioParam.GLIDER_MANUFACTURE, R.string.pref_key_glider_manufacture, VarioParamMapData.TYPE_STRING, ""),
        new VarioParamMapData(VarioParam.GLIDER_MODEL, R.string.pref_key_glider_model, VarioParamMapData.TYPE_STRING, ""),
        // IGCLogger
        new VarioParamMapData(VarioParam.LOGGER_ENABLE, R.string.pref_key_igc_enable, VarioParamMapData.TYPE_BOOLEAN, "1"),
        new VarioParamMapData(VarioParam.LOGGER_TAKEOFF_SPEED, R.string.pref_key_igc_takeoff_speed, VarioParamMapData.TYPE_INTEGER, "10"),
        new VarioParamMapData(VarioParam.LOGGER_LANDING_TIMEOUT, R.string.pref_key_igc_landing_timeout, VarioParamMapData.TYPE_INTEGER, "30000"),
        new VarioParamMapData(VarioParam.LOGGER_LOGGING_INTERVAL, R.string.pref_key_igc_logging_interval, VarioParamMapData.TYPE_INTEGER, "1000"),
        new VarioParamMapData(VarioParam.LOGGER_PILOT, R.string.pref_key_igc_pilot_name, VarioParamMapData.TYPE_STRING, ""),
        new VarioParamMapData(VarioParam.LOGGER_TIMEZONE, R.string.pref_key_igc_timezone, VarioParamMapData.TYPE_INTEGER, "9"),
        // VarioSettings
        new VarioParamMapData(VarioParam.VARIO_CLIMB_THRESHOLD, R.string.pref_key_vario_climb_threshold, VarioParamMapData.TYPE_FLOAT, "0.2"),
        new VarioParamMapData(VarioParam.VARIO_SINK_THRESHOLD, R.string.pref_key_vario_sink_threshold, VarioParamMapData.TYPE_FLOAT, "-3.0"),
        new VarioParamMapData(VarioParam.VARIO_SENSITIVITY, R.string.pref_key_vario_sensitivity, VarioParamMapData.TYPE_FLOAT, "0.1"),
        new VarioParamMapData(VarioParam.VARIO_SENTENCE, R.string.pref_key_vario_sentence, VarioParamMapData.TYPE_INTEGER, "0"),
        new VarioParamMapData(VarioParam.VARIO_BAROONLY, R.string.pref_key_vario_baro_only, VarioParamMapData.TYPE_BOOLEAN, "1"),
        // ToneTables
        new VarioParamMapData(VarioParam.TONETABLE_00_VELOCITY, R.string.pref_key_tone_00_velocity, VarioParamMapData.TYPE_FLOAT,"-10.0"),
        new VarioParamMapData(VarioParam.TONETABLE_00_FREQ, R.string.pref_key_tone_00_freq, VarioParamMapData.TYPE_FLOAT,	"200"),
        new VarioParamMapData(VarioParam.TONETABLE_00_PERIOD, R.string.pref_key_tone_00_period, VarioParamMapData.TYPE_INTEGER,"200"),
        new VarioParamMapData(VarioParam.TONETABLE_00_DUTY, R.string.pref_key_tone_00_duty, VarioParamMapData.TYPE_INTEGER,	"100"),
        new VarioParamMapData(VarioParam.TONETABLE_01_VELOCITY, R.string.pref_key_tone_01_velocity, VarioParamMapData.TYPE_FLOAT,"-3.0"),
        new VarioParamMapData(VarioParam.TONETABLE_01_FREQ, R.string.pref_key_tone_01_freq, VarioParamMapData.TYPE_FLOAT,	"293"),
        new VarioParamMapData(VarioParam.TONETABLE_01_PERIOD, R.string.pref_key_tone_01_period, VarioParamMapData.TYPE_INTEGER,"200"),
        new VarioParamMapData(VarioParam.TONETABLE_01_DUTY, R.string.pref_key_tone_01_duty, VarioParamMapData.TYPE_INTEGER,	"100"),
        new VarioParamMapData(VarioParam.TONETABLE_02_VELOCITY, R.string.pref_key_tone_02_velocity, VarioParamMapData.TYPE_FLOAT,"-2.0"),
        new VarioParamMapData(VarioParam.TONETABLE_02_FREQ, R.string.pref_key_tone_02_freq, VarioParamMapData.TYPE_FLOAT,	"369"),
        new VarioParamMapData(VarioParam.TONETABLE_02_PERIOD, R.string.pref_key_tone_02_period, VarioParamMapData.TYPE_INTEGER,"200"),
        new VarioParamMapData(VarioParam.TONETABLE_02_DUTY, R.string.pref_key_tone_02_duty, VarioParamMapData.TYPE_INTEGER,	"100"),
        new VarioParamMapData(VarioParam.TONETABLE_03_VELOCITY, R.string.pref_key_tone_03_velocity, VarioParamMapData.TYPE_FLOAT,"-1.0"),
        new VarioParamMapData(VarioParam.TONETABLE_03_FREQ, R.string.pref_key_tone_03_freq, VarioParamMapData.TYPE_FLOAT,	"440"),
        new VarioParamMapData(VarioParam.TONETABLE_03_PERIOD, R.string.pref_key_tone_03_period, VarioParamMapData.TYPE_INTEGER,"200"),
        new VarioParamMapData(VarioParam.TONETABLE_03_DUTY, R.string.pref_key_tone_03_duty, VarioParamMapData.TYPE_INTEGER,	"100"),
        new VarioParamMapData(VarioParam.TONETABLE_04_VELOCITY, R.string.pref_key_tone_04_velocity, VarioParamMapData.TYPE_FLOAT,"0.09"),
        new VarioParamMapData(VarioParam.TONETABLE_04_FREQ, R.string.pref_key_tone_04_freq, VarioParamMapData.TYPE_FLOAT,	"400"),
        new VarioParamMapData(VarioParam.TONETABLE_04_PERIOD, R.string.pref_key_tone_04_period, VarioParamMapData.TYPE_INTEGER,"600"),
        new VarioParamMapData(VarioParam.TONETABLE_04_DUTY, R.string.pref_key_tone_04_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        new VarioParamMapData(VarioParam.TONETABLE_05_VELOCITY, R.string.pref_key_tone_05_velocity, VarioParamMapData.TYPE_FLOAT,"0.10"),
        new VarioParamMapData(VarioParam.TONETABLE_05_FREQ, R.string.pref_key_tone_05_freq, VarioParamMapData.TYPE_FLOAT,	"400"),
        new VarioParamMapData(VarioParam.TONETABLE_05_PERIOD, R.string.pref_key_tone_05_period, VarioParamMapData.TYPE_INTEGER,"600"),
        new VarioParamMapData(VarioParam.TONETABLE_05_DUTY, R.string.pref_key_tone_05_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        new VarioParamMapData(VarioParam.TONETABLE_06_VELOCITY, R.string.pref_key_tone_06_velocity, VarioParamMapData.TYPE_FLOAT,"1.98"),
        new VarioParamMapData(VarioParam.TONETABLE_06_FREQ, R.string.pref_key_tone_06_freq, VarioParamMapData.TYPE_FLOAT,	"499"),
        new VarioParamMapData(VarioParam.TONETABLE_06_PERIOD, R.string.pref_key_tone_06_period, VarioParamMapData.TYPE_INTEGER,"552"),
        new VarioParamMapData(VarioParam.TONETABLE_06_DUTY, R.string.pref_key_tone_06_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        new VarioParamMapData(VarioParam.TONETABLE_07_VELOCITY, R.string.pref_key_tone_07_velocity, VarioParamMapData.TYPE_FLOAT,"3.14"),
        new VarioParamMapData(VarioParam.TONETABLE_07_FREQ, R.string.pref_key_tone_07_freq, VarioParamMapData.TYPE_FLOAT,	"868"),
        new VarioParamMapData(VarioParam.TONETABLE_07_PERIOD, R.string.pref_key_tone_07_period, VarioParamMapData.TYPE_INTEGER,"347"),
        new VarioParamMapData(VarioParam.TONETABLE_07_DUTY, R.string.pref_key_tone_07_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        new VarioParamMapData(VarioParam.TONETABLE_08_VELOCITY, R.string.pref_key_tone_08_velocity, VarioParamMapData.TYPE_FLOAT,"4.57"),
        new VarioParamMapData(VarioParam.TONETABLE_08_FREQ, R.string.pref_key_tone_08_freq, VarioParamMapData.TYPE_FLOAT,	"1084"),
        new VarioParamMapData(VarioParam.TONETABLE_08_PERIOD, R.string.pref_key_tone_08_period, VarioParamMapData.TYPE_INTEGER,"262"),
        new VarioParamMapData(VarioParam.TONETABLE_08_DUTY, R.string.pref_key_tone_08_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        new VarioParamMapData(VarioParam.TONETABLE_09_VELOCITY, R.string.pref_key_tone_09_velocity, VarioParamMapData.TYPE_FLOAT,"6.28"),
        new VarioParamMapData(VarioParam.TONETABLE_09_FREQ, R.string.pref_key_tone_09_freq, VarioParamMapData.TYPE_FLOAT,	"1354"),
        new VarioParamMapData(VarioParam.TONETABLE_09_PERIOD, R.string.pref_key_tone_09_period, VarioParamMapData.TYPE_INTEGER,"185"),
        new VarioParamMapData(VarioParam.TONETABLE_09_DUTY, R.string.pref_key_tone_09_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        new VarioParamMapData(VarioParam.TONETABLE_10_VELOCITY, R.string.pref_key_tone_10_velocity, VarioParamMapData.TYPE_FLOAT,"8.15"),
        new VarioParamMapData(VarioParam.TONETABLE_10_FREQ, R.string.pref_key_tone_10_freq, VarioParamMapData.TYPE_FLOAT,	"1593"),
        new VarioParamMapData(VarioParam.TONETABLE_10_PERIOD, R.string.pref_key_tone_10_period, VarioParamMapData.TYPE_INTEGER,"168"),
        new VarioParamMapData(VarioParam.TONETABLE_10_DUTY, R.string.pref_key_tone_10_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        new VarioParamMapData(VarioParam.TONETABLE_11_VELOCITY, R.string.pref_key_tone_11_velocity, VarioParamMapData.TYPE_FLOAT,"10.00"),
        new VarioParamMapData(VarioParam.TONETABLE_11_FREQ, R.string.pref_key_tone_11_freq, VarioParamMapData.TYPE_FLOAT,	"1800"),
        new VarioParamMapData(VarioParam.TONETABLE_11_PERIOD, R.string.pref_key_tone_11_period, VarioParamMapData.TYPE_INTEGER,"150"),
        new VarioParamMapData(VarioParam.TONETABLE_11_DUTY, R.string.pref_key_tone_11_duty, VarioParamMapData.TYPE_INTEGER,	"50"),
        // VolumeSettings
        new VarioParamMapData(VarioParam.VOLUME_VARIO, R.string.pref_key_volume_vario, VarioParamMapData.TYPE_INTEGER, "80"),
        new VarioParamMapData(VarioParam.VOLUME_EFFECT, R.string.pref_key_volume_effect, VarioParamMapData.TYPE_INTEGER, "5"),
        // ThresholdSettings
        new VarioParamMapData(VarioParam.THRESHOLD_LOW_BATTERY, R.string.pref_key_threshold_low_battery, VarioParamMapData.TYPE_FLOAT, "2.8"),
        new VarioParamMapData(VarioParam.THRESHOLD_SHUTDOWN_HOLDTIME, R.string.pref_key_threshold_shutdown_holdtime, VarioParamMapData.TYPE_INTEGER, "1000"),
        new VarioParamMapData(VarioParam.THRESHOLD_AUTO_SHUTDOWN_VARIO, R.string.pref_key_auto_shutdown_vario, VarioParamMapData.TYPE_INTEGER, "6000000"),
        new VarioParamMapData(VarioParam.THRESHOLD_AUTO_SHUTDOWN_UMS, R.string.pref_key_auto_shutdown_ums, VarioParamMapData.TYPE_INTEGER, "6000000"),
        // KalmanParameters
        new VarioParamMapData(VarioParam.KALMAN_VAR_ZMEAS, R.string.pref_key_kalman_zmeas, VarioParamMapData.TYPE_FLOAT, "400.0"),
        new VarioParamMapData(VarioParam.KALMAN_VAR_ZACCEL, R.string.pref_key_kalman_zaccel, VarioParamMapData.TYPE_FLOAT, "1000.0"),
        new VarioParamMapData(VarioParam.KALMAN_VAR_ACCELBIAS, R.string.pref_key_kalman_accelbias, VarioParamMapData.TYPE_FLOAT, "1.0"),
        //new VarioParamMapData(VarioParam.KALMAN_SIGMA_P, R.string.pref_key_kalman_sigma_p, VarioParamMapData.TYPE_FLOAT, "0.1"),
        //new VarioParamMapData(VarioParam.KALMAN_SIGMA_A, R.string.pref_key_kalman_sigma_a, VarioParamMapData.TYPE_FLOAT, "0.3"),
        // CalibrationData
        new VarioParamMapData(VarioParam.CALDATA_ACCEL_00, R.string.pref_key_caldata_accel_x, VarioParamMapData.TYPE_FLOAT, "0.0"),
        new VarioParamMapData(VarioParam.CALDATA_ACCEL_01, R.string.pref_key_caldata_accel_y, VarioParamMapData.TYPE_FLOAT, "0.0"),
        new VarioParamMapData(VarioParam.CALDATA_ACCEL_02, R.string.pref_key_caldata_accel_z, VarioParamMapData.TYPE_FLOAT, "0.0"),
        new VarioParamMapData(VarioParam.CALDATA_GYRO_00, R.string.pref_key_caldata_gyro_x, VarioParamMapData.TYPE_FLOAT, "0.0"),
        new VarioParamMapData(VarioParam.CALDATA_GYRO_01, R.string.pref_key_caldata_gyro_y, VarioParamMapData.TYPE_FLOAT, "0.0"),
        new VarioParamMapData(VarioParam.CALDATA_GYRO_02, R.string.pref_key_caldata_gyro_z, VarioParamMapData.TYPE_FLOAT, "0.0"),
        };
    }
}
