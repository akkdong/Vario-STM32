package club.rascal.notorious.aconsole;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.support.v7.app.ActionBar;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.RingtonePreference;
import android.text.TextUtils;
import android.util.Log;
import android.view.MenuItem;

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
public class VarioPreference extends AppCompatPreferenceActivity {

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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Intent intent = getIntent();
        intent.putExtra(PreferenceActivity.EXTRA_SHOW_FRAGMENT, VarioPreferenceFragment.class.getName());

        super.onCreate(savedInstanceState);
        setupActionBar();
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

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            //if (!super.onMenuItemSelected(featureId, item)) {
            //    NavUtils.navigateUpFromSameTask(this);
            //}
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
     * This fragment shows general preferences only. It is used when the
     * activity is showing a two-pane settings UI.
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    public static class VarioPreferenceFragment extends PreferenceFragment {
        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pref_vario);
            setHasOptionsMenu(true);

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

            /*
            Preference prefToneEditor = (Preference)findPreference("pref_title_tone_editor");
            prefToneEditor.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    startActivity(new Intent(CurrentActivity.this, ToneEditorActivity.class));
                    return true;
                }
            });
            Preference prefCalibration = (Preference)findPreference("pref_title_calibration");
            prefCalibration.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    startActivity(new Intent(this, CalibrationActivity.class));
                    return true;
                }
            });
            */
        }

        @Override
        public boolean onOptionsItemSelected(MenuItem item) {
            int id = item.getItemId();
            if (id == android.R.id.home) {
                startActivity(new Intent(getActivity(), VarioPreference.class));
                return true;
            }
            return super.onOptionsItemSelected(item);
        }
    }
}
