package club.rascal.notorious.aconsole;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;

import app.akexorcist.bluetoothspp.library.BluetoothSPP;
import app.akexorcist.bluetoothspp.library.BluetoothState;
import app.akexorcist.bluetoothspp.library.DeviceList;

public class MainActivity extends AppCompatActivity implements VarioAgent.VarioListener, BluetoothSPP.BluetoothConnectionListener {

    private VarioAgent mAgent;
    private Toolbar mToolbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mToolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(mToolbar);

        // get singleton instance
        mAgent = VarioAgent.getInstance();
        // initialize vario-agnent
        mAgent.init(this, (BluetoothSPP.BluetoothConnectionListener)this);
        // bluetooth must be available
        if (! mAgent.isBluetoothAvailable()) {
            Toast.makeText(this, R.string.toast_bt_not_available, Toast.LENGTH_SHORT).show();
            finish();
        }
        // receive all sensor data : gps, vario, imu(acc,gyro,baro)
        mAgent.setVarioListener(this, VarioAgent.ListenerFilter.FILTER_DATA | VarioAgent.ListenerFilter.FILTER_RESPONSE, (VarioAgent.VarioListener)this);
    }

    @Override
    public void onStart() {
        super.onStart();

        if (! mAgent.isBluetoothEnabled()) {
            Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(intent, BluetoothState.REQUEST_ENABLE_BT);
        } else {
            if (! mAgent.isServiceAvailable()) {
                mAgent.startService();

                //setup();
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        mAgent.unsetVarioListener(this);
        mAgent.stopService();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        //
        updateMenu();

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_connect:
                if (mAgent.getServiceState() == BluetoothState.STATE_CONNECTED) {
                    mAgent.disconnect();
                } else {
                    startActivityForResult(new Intent(this, DeviceList.class), BluetoothState.REQUEST_CONNECT_DEVICE);
                }
                return true;

            case R.id.action_settings:
                if (mAgent.getServiceState() == BluetoothState.STATE_CONNECTED) {
                    //Intent intent = new Intent(this, VarioPreference.class);
                    startActivity(new Intent(this, VarioPreference.class));
                } else {
                    Toast.makeText(this, R.string.toast_bt_not_connected, Toast.LENGTH_SHORT).show();
                }
                return true;

            case R.id.action_dump_acc:
                item.setChecked(! item.isChecked());
                return true;

            case R.id.action_dump_gyro:
                item.setChecked(! item.isChecked());
                return true;

            case R.id.action_dump_pressure:
                item.setChecked(! item.isChecked());
                return true;

            case R.id.action_calibration:
                if (mAgent.getServiceState() == BluetoothState.STATE_CONNECTED) {
                    startActivity(new Intent(this, CalibrationActivity.class));
                } else {
                    Toast.makeText(this, R.string.toast_bt_not_connected, Toast.LENGTH_SHORT).show();
                }
                return true;

            default:
                // If we got here, the user's action was not recognized.
                // Invoke the superclass to handle it.
                return super.onOptionsItemSelected(item);

        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == BluetoothState.REQUEST_CONNECT_DEVICE) {
            if(resultCode == Activity.RESULT_OK)
                mAgent.connect(data);
        } else if (requestCode == BluetoothState.REQUEST_ENABLE_BT) {
            if (resultCode == Activity.RESULT_OK) {
                mAgent.startService();
                //setup();
            } else {
                Toast.makeText(this,  R.string.toast_bt_not_enabled, Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }

    @Override
    public void onDeviceConnected(String name, String address) {
        Toast.makeText(this, R.string.toast_bt_connected, Toast.LENGTH_SHORT).show();

        VarioCommand dp = new VarioCommand(VarioCommand.CMD_DUMP_PARAMETERS);
        mAgent.send(dp);

        //
        updateMenu();
    }

    @Override
    public void onDeviceDisconnected() {
        Toast.makeText(this, R.string.toast_bt_disconnected, Toast.LENGTH_SHORT).show();

        //
        updateMenu();
    }

    @Override
    public void onDeviceConnectionFailed() {
        Toast.makeText(this, R.string.toast_bt_connection_failed, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onDataReceived(AbstractData data) {
        // ...
        if (data.mType == AbstractData.DATA_VARIO) {
            VarioData vario = (VarioData)data;

            TextView viewPressure = (TextView)findViewById(R.id.valuePressure);
            viewPressure.setText(String.format("%.2f ㎩", vario.mPressure));

            TextView viewAltitude = (TextView)findViewById(R.id.valueBaroAltitude);
            viewAltitude.setText(String.format("%.0f m", vario.mAltitude));

            TextView viewVario = (TextView)findViewById(R.id.valueVario);
            viewVario.setText(String.format("%.2f ㎧", vario.mVario / 100));

            TextView viewTemperature = (TextView)findViewById(R.id.valueTemperature);
            viewTemperature.setText(String.format("%.1f ℃", vario.mTemperature));

            TextView viewBattery = (TextView)findViewById(R.id.valueBattery);
            viewBattery.setText(String.format("%.1f Volt", vario.mBattery));
        } else if (data.mType == AbstractData.DATA_LOCATION) {
            LocationData loc = (LocationData)data;

            TextView viewLatitude = (TextView)findViewById(R.id.valueLatitude);
            viewLatitude.setText(getCoordinateString(loc.mLatitude, true));

            TextView viewLongitude = (TextView)findViewById(R.id.valueLongitude);
            viewLongitude.setText(getCoordinateString(loc.mLongitude, false));

            TextView viewAltitude = (TextView)findViewById(R.id.valueAltitude);
            viewAltitude.setText(String.format("%.0f m", loc.mAltitude));

            TextView viewSpeed = (TextView)findViewById(R.id.valueSpeed);
            viewSpeed.setText(String.format("%.0f Km/s", loc.mSpeed));

            TextView viewTrack = (TextView)findViewById(R.id.valueTrack);
            viewTrack.setText(String.format("%.0f degree", loc.mTrackangle));
        }
    }

    private String getCoordinateString(double coord, boolean lat) {
        char subfix = lat ? 'N' : 'E';

        if (coord < 0) {
            subfix = lat ? 'S' : 'W';
            coord = -coord;
        }

        int degree = (int)(coord / 100);
        coord = coord - (degree * 100);

        int minute = (int)coord;
        double second = (coord - minute) * 60;

        return String.format("%d˚%02d˙%02.2f %c", degree, minute, second, subfix);
    }

    @Override
    public void onResponseReceived(VarioResponse response) {
        // ...
        Log.i("Vario", "onResponseReceived : " + response.toString());
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);

        switch (response.mCode) {
            case VarioResponse.RCODE_DUMP_PARAM :
                saveParameter(response);
                break;
            case VarioResponse.RCODE_UPDATE_PARAM :
                break;
        }
    }

    private void saveParameter(VarioResponse response) {

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
                        //editor.putInt(getString(map.mPrefName), response.getInteger());
                        editor.putString(getString(map.mPrefName), Integer.toString(response.getInteger()));
                        editor.apply();
                        break;
                    case VarioParamMapData.TYPE_FLOAT:
                        //editor.putFloat(getString(map.mPrefName), response.getFloat());
                        editor.putString(getString(map.mPrefName), Float.toString(response.getFloat()));
                        editor.apply();
                        break;
                    case VarioParamMapData.TYPE_STRING:
                        editor.putString(getString(map.mPrefName), response.getString());
                        editor.apply();
                        break;
                }

                //Log.i("Vario", "   -> " + pref.getString(getString(map.mPrefName), ""));
            }
        }
    }

    private void updateMenu() {
        Menu menu = ((Toolbar)findViewById(R.id.toolbar)).getMenu();

        if (mAgent.getServiceState() == BluetoothState.STATE_CONNECTED) {
            MenuItem item = menu.findItem(R.id.action_calibration);
            if (item != null) {
                item.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
                //item.setVisible(true);
            }
        } else {
            MenuItem item = menu.findItem(R.id.action_calibration);
            if (item != null) {
                item.setShowAsAction(MenuItem.SHOW_AS_ACTION_NEVER);
                //item.setVisible(false);
            }
        }
    }
}
