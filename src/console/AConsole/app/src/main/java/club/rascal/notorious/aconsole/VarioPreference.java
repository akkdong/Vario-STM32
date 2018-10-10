package club.rascal.notorious.aconsole;

import android.annotation.TargetApi;
import android.app.Fragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.support.v7.app.ActionBar;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import java.util.LinkedList;
import java.util.List;

/**
 * A {@link PreferenceActivity} that presents a set of application settings. On
 * handset devices, settings are presented as a single list. On tablets,
 * settings are split by category, with category headers shown to the left of
 * the list of settings.
 * <p>
 * See <a href="http://developer.android.com/design/patterns/settings.html">
 * Android Design: Settings</a> for design guidelines and the <a
 * href="http://developer.android.com/guide/topics/ui/settings.html">Settings
 * API Guide</a> for more information on developing a Settings UI.
 */
public class VarioPreference extends AppCompatPreferenceActivity implements VarioAgent.VarioListener {

    //
    private VarioAgent mAgent = null;
    private Fragment mActiveFragment = null;

    private LinkedList<VarioCommand> mCommands = null;

    /**
     *
     */
    protected interface IPreferenceFragment {
        void refreshFragment();
    }

    /**
     * A preference value change listener that updates the preference's summary
     * to reflect its new value.
     */
    private static Preference.OnPreferenceChangeListener sBindPreferenceSummaryToValueListener = new Preference.OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object value) {
            String stringValue = value.toString();
            Log.i("Vario", "Preference : " + stringValue);

            if (preference instanceof ListPreference) {
                // For list preferences, look up the correct display value in
                // the preference's 'entries' list.
                ListPreference listPreference = (ListPreference) preference;
                int index = listPreference.findIndexOfValue(stringValue);

                // Set the summary to reflect the new value.
                preference.setSummary(
                        index >= 0
                                ? listPreference.getEntries()[index]
                                : null);
            } else {
                // For all other preferences, set the summary to the value's
                // simple string representation.
                preference.setSummary(stringValue);
            }

            return true;
        }
    };

    /**
     * Helper method to determine if the device has an extra-large screen. For
     * example, 10" tablets are extra-large.
     */
    private static boolean isXLargeTablet(Context context) {
        return (context.getResources().getConfiguration().screenLayout
                & Configuration.SCREENLAYOUT_SIZE_MASK) >= Configuration.SCREENLAYOUT_SIZE_XLARGE;
    }

    /**
     * Binds a preference's summary to its value. More specifically, when the
     * preference's value is changed, its summary (line of text below the
     * preference title) is updated to reflect the value. The summary is also
     * immediately updated upon calling this method. The exact display format is
     * dependent on the type of preference.
     *
     * @see #sBindPreferenceSummaryToValueListener
     */
    private static void bindPreferenceSummaryToValue(Preference preference) {
        // Set the listener to watch for value changes.
        preference.setOnPreferenceChangeListener(sBindPreferenceSummaryToValueListener);

        // Trigger the listener immediately with the preference's
        // current value.
        sBindPreferenceSummaryToValueListener.onPreferenceChange(preference,
                PreferenceManager
                        .getDefaultSharedPreferences(preference.getContext())
                        .getString(preference.getKey(), ""));
    }

    /**
     *
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Intent intent = getIntent();
        intent.putExtra(PreferenceActivity.EXTRA_SHOW_FRAGMENT, VarioPreferenceFragment.class.getName());

        super.onCreate(savedInstanceState);
        setupActionBar();

        // get VarioAgent instance
        mAgent = VarioAgent.getInstance();
        // receive vario command response
        mAgent.setVarioListener(this, VarioAgent.ListenerFilter.FILTER_RESPONSE, (VarioAgent.VarioListener)this);
        // request all device's parameters
        mAgent.send(new VarioCommand(VarioCommand.CMD_DUMP_PROPERTY));
    }

    /**
     * Set up the {@link android.app.ActionBar}, if the API is available.
     */
    private void setupActionBar() {
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null) {
            // Show the Up button in the action bar.
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    /**
     *
     */
    @Override
    protected void onResume() {
        super.onResume();

        // activity resumed! -> enable listener
        mAgent.enableVarioListener(this);
    }

    /**
     *
     */
    protected void onPause() {
        super.onPause();

        // activity paused! -> disable listener
        mAgent.disableVarioListener(this);
    }

    /**
     *
     */
    @Override
    protected void onDestroy() {
        //
        mAgent.unsetVarioListener(this);
        mAgent = null;

        super.onDestroy();
    }

    /**
     *
     * @param fragment
     */
    @Override
    public void onAttachFragment(Fragment fragment) {
        mActiveFragment = fragment;
    }

    /**
     *
     * @param menu
     * @return
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_pref, menu);

        return true;
    }

    /**
     *
     * @param item
     * @return
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_upload_preference :
                Log.i("Vario", "VarioPreference.onOptionSelected() -> action_upload_preference");

                // reload ?
                new AlertDialog.Builder(this)
                        .setTitle(R.string.title_alert_reload_preference)
                        .setMessage(R.string.message_reload_preference)
                        .setIcon(android.R.drawable.ic_dialog_info)
                        .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                // request device's paramters -> refresh prefereces to the device's parameters
                                VarioCommand dp = new VarioCommand(VarioCommand.CMD_DUMP_PROPERTY);
                                mAgent.send(dp);
                            }
                        })
                        .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                // nop
                            }
                        }).show();

                return true;

            case R.id.action_dnload_preference :
                Log.i("Vario", "VarioPreference.onOptionSelected() -> action_dnload_preference");

                // ? confirm
                new AlertDialog.Builder(this)
                        .setTitle(R.string.title_alert_download_preference)
                        .setMessage(R.string.message_download_preference)
                        .setIcon(android.R.drawable.ic_dialog_info)
                        .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialogInterface, int i) {
                                //
                                generateUpdateCommands();
                                sendUpdateCommands();
                            }
                        }).show();

                return true;

            default:
                // If we got here, the user's action was not recognized.
                // Invoke the superclass to handle it.
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onMenuItemSelected(featureId, item);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean onIsMultiPane() {
        return isXLargeTablet(this);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public void onBuildHeaders(List<Header> target) {
        loadHeadersFromResource(R.xml.pref_headers, target);
    }

    /**
     * This method stops fragment injection in malicious applications.
     * Make sure to deny any unknown fragments here.
     */
    protected boolean isValidFragment(String fragmentName) {
        return PreferenceFragment.class.getName().equals(fragmentName) || VarioPreferenceFragment.class.getName().equals(fragmentName);
    }

    /**
     *
     */
    protected void generateUpdateCommands() {
        //
        SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(this);

        // ...
        mCommands = new LinkedList<VarioCommand>();

        // GliderInfo
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.GLIDER_TYPE, getLongPreference(pref,R.string.pref_key_glider_type, "1")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.GLIDER_MANUFACTURE, getStringPreference(pref, R.string.pref_key_glider_manufacture, "")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.GLIDER_MODEL, getStringPreference(pref, R.string.pref_key_glider_model, "")));
        // IGC-Logger
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.LOGGER_ENABLE, pref.getBoolean(getString(R.string.pref_key_igc_enable), true) ? 1 : 0));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.LOGGER_TAKEOFF_SPEED, getLongPreference(pref, R.string.pref_key_igc_takeoff_speed, "10")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.LOGGER_LANDING_TIMEOUT, getLongPreference(pref, R.string.pref_key_igc_landing_timeout, "30000")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.LOGGER_LOGGING_INTERVAL, getLongPreference(pref, R.string.pref_key_igc_logging_interval, "1000")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.LOGGER_PILOT, getStringPreference(pref, R.string.pref_key_igc_pilot_name, "")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.LOGGER_TIMEZONE, getLongPreference(pref, R.string.pref_key_igc_timezone, "9")));
        // Vario Settings
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.VARIO_CLIMB_THRESHOLD, getDoublePreference(pref, R.string.pref_key_vario_climb_threshold, "0.2")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.VARIO_SINK_THRESHOLD, getDoublePreference(pref, R.string.pref_key_vario_sink_threshold, "-3.0")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.VARIO_SENSITIVITY, getDoublePreference(pref, R.string.pref_key_vario_sensitivity, "0.1")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.VARIO_SENTENCE, getLongPreference(pref, R.string.pref_key_vario_sentence, "0")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.VARIO_BAROONLY, pref.getBoolean(getString(R.string.pref_key_vario_baro_only), true) ? 1 : 0));
        // Volume Settings
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.VOLUME_VARIO, getLongPreference(pref, R.string.pref_key_volume_vario, "80")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.VOLUME_EFFECT, getLongPreference(pref, R.string.pref_key_volume_effect, "5")));
        // Threshold Settings
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.THRESHOLD_LOW_BATTERY, getDoublePreference(pref, R.string.pref_key_threshold_low_battery, "2.8")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.THRESHOLD_SHUTDOWN_HOLDTIME, getLongPreference(pref, R.string.pref_key_threshold_shutdown_holdtime, "1000")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.THRESHOLD_AUTO_SHUTDOWN_VARIO, getLongPreference(pref, R.string.pref_key_auto_shutdown_vario, "600000")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.THRESHOLD_AUTO_SHUTDOWN_UMS, getLongPreference(pref, R.string.pref_key_auto_shutdown_ums, "600000")));
        // Kalman Parameters
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.KALMAN_VAR_ZMEAS, getDoublePreference(pref, R.string.pref_key_kalman_zmeas, "400.0")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.KALMAN_VAR_ZACCEL, getDoublePreference(pref, R.string.pref_key_kalman_zaccel, "1000.0")));
        mCommands.add(new VarioCommand(VarioCommand.CMD_UPDATE_PROPERTY, VarioParam.KALMAN_VAR_ACCELBIAS, getDoublePreference(pref, R.string.pref_key_kalman_accelbias, "1.0")));

        // save parameters to eeprom
        mCommands.add(new VarioCommand(VarioCommand.CMD_SAVE_PROPERTY));
    }

    private long getLongPreference(SharedPreferences pref, int id, String strDefault) {
        return Long.parseLong(pref.getString(getString(id), strDefault));
    }

    private double getDoublePreference(SharedPreferences pref, int id, String strDefault) {
        return Double.parseDouble(pref.getString(getString(id), strDefault));
    }

    private String getStringPreference(SharedPreferences pref, int id, String strDefault) {
        return pref.getString(getString(id), strDefault);
    }

    /**
     *
     */
    protected void sendUpdateCommands() {
        if (mCommands != null && ! mCommands.isEmpty()) {
            mAgent.send(mCommands.removeFirst());
        }
    }

    /**
     * This fragment shows general preferences only. It is used when the
     * activity is showing a two-pane settings UI.
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class VarioPreferenceFragment extends PreferenceFragment implements IPreferenceFragment {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_vario);
            setHasOptionsMenu(true);

            //
            bindPreferenceSummary();
        }

        @Override
        public boolean onOptionsItemSelected(MenuItem item) {
             switch (item.getItemId()) {
                case  android.R.id.home :
                    startActivity(new Intent(getActivity(), VarioPreference.class));
                    return true;
            }

            return super.onOptionsItemSelected(item);
        }

        @Override
        public void refreshFragment() {
            setPreferenceScreen(null);
            addPreferencesFromResource(R.xml.pref_vario);

            bindPreferenceSummary();
        }

        private void bindPreferenceSummary() {
            // Bind the summaries of EditText/List/Dialog/Ringtone preferences
            // to their values. When their values change, their summaries are
            // updated to reflect the new value, per the Android Design
            // guidelines.
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_glider_type)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_glider_manufacture)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_glider_model)));

            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_igc_takeoff_speed)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_igc_landing_timeout)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_igc_logging_interval)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_igc_pilot_name)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_igc_timezone)));

            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_vario_climb_threshold)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_vario_sink_threshold)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_vario_sensitivity)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_vario_sentence)));

            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_volume_vario)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_volume_effect)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_threshold_low_battery)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_threshold_shutdown_holdtime)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_auto_shutdown_vario)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_auto_shutdown_ums)));

            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_kalman_zmeas)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_kalman_zaccel)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_kalman_accelbias)));
            //bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_kalman_sigma_p)));
            //bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_kalman_sigma_a)));

            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_caldata_accel_x)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_caldata_accel_y)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_caldata_accel_z)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_caldata_gyro_x)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_caldata_gyro_y)));
            bindPreferenceSummaryToValue(findPreference(getString(R.string.pref_key_caldata_gyro_z)));
        }
    }

    @Override
    public void onDataReceived(AbstractData data) {
        // nop
        Log.i("Vario", "VarioPreference.onDataReceived: " + data.toString());
    }

    @Override
    public void onResponseReceived(VarioResponse response) {
        // ...
        Log.i("Vario", "VarioPreference.onResponseReceived: " + response.toString());

        switch (response.mCode) {
            case VarioCommand.CMD_DUMP_PROPERTY :
                //
                saveParameter(response);

                //
                if (response.mParam == VarioParam.EOF) {
                    if (mActiveFragment != null) {
                        ((IPreferenceFragment) mActiveFragment).refreshFragment();

                        Toast.makeText(this, R.string.toast_pref_reload_success, Toast.LENGTH_SHORT).show();
                    }
                }
                break;

            case VarioCommand.CMD_UPDATE_PROPERTY :
                /*
                if (response.mParam != VarioResponse.RPARAM_SUCCESS) {
                    // something wroing!! -> remove all VarioCommand & alert
                    mCommands.clear();
                    mCommands = null;

                    // alert?
                    Toast.makeText(this, R.string.toast_pref_download_failed, Toast.LENGTH_SHORT).show();
                }
                else {
                    // send next if exists
                    sendUpdateCommands();
                }
                */
                // send next if exists
                sendUpdateCommands();
                break;

            case VarioCommand.CMD_SAVE_PROPERTY:
                if (response.mParam == VarioResponse.RPARAM_SUCCESS) {
                    Toast.makeText(this, R.string.toast_pref_download_success, Toast.LENGTH_SHORT).show();
                }
                // else nop : CMD_SAVE_PROPERTY always returns RPARAM_SUCCESS
                break;
        }
    }

    private void saveParameter(VarioResponse response) {
        //
        for (VarioParamMapData map : VarioParamMaps.mMaps) {
            if (map.mParam == response.mParam) {
                SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(this);
                SharedPreferences.Editor editor = pref.edit();

                switch (map.mType) {
                    case VarioParamMapData.TYPE_BOOLEAN :
                        editor.putBoolean(getString(map.mPrefName), response.getInteger() != 0);
                        editor.apply();
                        break;
                    case VarioParamMapData.TYPE_INTEGER:
                        editor.putString(getString(map.mPrefName), Integer.toString(response.getInteger()));
                        editor.apply();
                        break;
                    case VarioParamMapData.TYPE_FLOAT:
                        editor.putString(getString(map.mPrefName), Float.toString(response.getFloat()));
                        editor.apply();
                        break;
                    case VarioParamMapData.TYPE_STRING:
                        editor.putString(getString(map.mPrefName), response.getString());
                        editor.apply();
                        break;
                }
            }
        }
    }
}
