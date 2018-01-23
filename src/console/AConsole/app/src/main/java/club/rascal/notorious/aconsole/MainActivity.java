package club.rascal.notorious.aconsole;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import app.akexorcist.bluetoothspp.library.BluetoothSPP;
import app.akexorcist.bluetoothspp.library.BluetoothState;
import app.akexorcist.bluetoothspp.library.DeviceList;
import club.rascal.notorious.aconsole.VarioAgent.VarioListener;

public class MainActivity extends AppCompatActivity {

    private VarioAgent agent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        agent = VarioAgent.getInstance();

        agent.init(this, new BluetoothSPP.BluetoothConnectionListener() {
            @Override
            public void onDeviceConnected(String name, String address) {
                Toast.makeText(getApplicationContext(), R.string.toast_bt_connected, Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onDeviceDisconnected() {
                Toast.makeText(getApplicationContext(), R.string.toast_bt_disconnected, Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onDeviceConnectionFailed() {
                Toast.makeText(getApplicationContext(), R.string.toast_bt_connection_failed, Toast.LENGTH_SHORT).show();
            }
        });

        if (! agent.isBluetoothAvailable()) {
            Toast.makeText(getApplicationContext(), R.string.toast_bt_not_available, Toast.LENGTH_SHORT).show();
            finish();
        }

        agent.setVarioListener(this, VarioAgent.VarioListenerFilter.FILTER_GPS, new VarioListener() {
            @Override
            public void onGPSUpdated(float latitude, float longigute, float altitude, float speed, float trackAngle) {

            }

            @Override
            public void onVarioUpdated(float vario, float pressure, float temperature, float voltage) {
                Log.i("Vario", "onVarioUpdated");
            }

            @Override
            public void onAccelerometerUpdated(float ax, float ay, float az) {

            }

            @Override
            public void onGyrometerUpdated(float gx, float gy, float gz) {

            }

            @Override
            public void onPressureUpdated(float prs) {

            }
        });
    }

    @Override
    public void onStart() {
        super.onStart();

        if (! agent.isBluetoothEnabled()) {
            Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(intent, BluetoothState.REQUEST_ENABLE_BT);
        } else {
            if (! agent.isServiceAvailable()) {
                agent.startService();

                //setup();
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        agent.unsetVarioListener(this);
        agent.stopService();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_connect:
                if (agent.getServiceState() == BluetoothState.STATE_CONNECTED) {
                    agent.disconnect();
                } else {
                    Intent intent = new Intent(getApplicationContext(), DeviceList.class);
                    startActivityForResult(intent, BluetoothState.REQUEST_CONNECT_DEVICE);
                }
                return true;

            case R.id.action_settings:
                 return true;

            case R.id.action_calibration:
                return true;

            case R.id.action_show_acc:
                return true;

            case R.id.action_show_gyro:
                return true;

            case R.id.action_show_baro:
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
                agent.connect(data);
        } else if (requestCode == BluetoothState.REQUEST_ENABLE_BT) {
            if (resultCode == Activity.RESULT_OK) {
                agent.startService();
                //setup();
            } else {
                Toast.makeText(getApplicationContext(),  R.string.toast_bt_not_enabled, Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }
}
