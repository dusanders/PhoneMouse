package zena.phonemouse;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;


public class MainActivity extends ActionBarActivity {
    //constants to send to Server for defining data
    private final int MOUSE_EVENT = 1;
    private final int KEYBOARD_EVENT = 2;
    private final int MOUSE_LEFT_CLICK = 1;
    private final int KEY_BACKSPACE = 8;
    private final int MOUSE_RIGHT_CLICK = 1;
    //values used during 'mouse' movement
    private float previousX = 0;
    private float previousY = 0;
    private float dx = 0;
    private float dy = 0;
    //values for connection to sever
    private String subnetString = "192.168.0";
    private ArrayList<String> availableServers;
    private InetAddress serverIp;
    private int serverPortNumber = 6666;
    //need to keep track of selections within available
    //  servers listView in SettingsView
    private View previousListViewSelection;
    //values used to track which view we are in
    private boolean inKeyboardView = false;
    private boolean inSettingsView = false;
    //handler for communication with server
    private UDPHandler udpHandler;
    //keep track of our connection status
    private String statusString = "Disconnected";
    //location of saved state
    private SharedPreferences savedState;
    private String SAVE_FILE_LOCATION = "com.zena.PhoneMouse.SAVED_STATE";
    //called when app is first created
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //open the array of available servers
        availableServers = new ArrayList<String>();
        savedState = this.getApplicationContext().getSharedPreferences(SAVE_FILE_LOCATION, Context.MODE_PRIVATE);
        if(savedState.contains("ServerIp")) {
            try {
                String savedIP = savedState.getString("ServerIP", null);
                serverPortNumber = savedState.getInt("ServerPort", 0);
                serverIp = InetAddress.getByName(savedIP);
                udpHandler = new UDPHandler(serverIp, serverPortNumber);
            } catch (UnknownHostException e) {
                e.printStackTrace();
            } catch (SocketException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        //call to setup main layout
        setMainDisplay();
    }

    @Override
    public void onStop() {
        super.onStop();
        SharedPreferences.Editor editor = savedState.edit();
        editor.putString("ServerIp", serverIp.getHostAddress());
        editor.putInt("ServerPort", serverPortNumber);
        editor.commit();
        udpHandler.disconnect();
    }
    //  Method to setup main layout of app
    private void setMainDisplay() {
        setContentView(R.layout.activity_main);
        Button keyboardButton = (Button) findViewById(R.id.KeyboardButton);
        Button leftClickButton = (Button) findViewById(R.id.LeftClickButton);
        Button rightClickButton = (Button) findViewById(R.id.RightClickButton);
        TextView connectionStatus = (TextView) findViewById(R.id.ConnectStatus);
        connectionStatus.setText(statusString);
        //call to setup the keyboard button action, we will need to inflate
        //  a view.
        setupKeyboardButton(keyboardButton);
        setupMouseButtons(leftClickButton, rightClickButton);
    }

    //  Method to capture touch events from user
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        //only continue if we are in main layout
        if(!inSettingsView && !inKeyboardView && udpHandler != null) {
            int scrollUp = 0;
            int scrollDown = 0;
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                //save the start position of the movement
                previousX = event.getX();
                previousY = event.getY();
            }
            if (event.getAction() == MotionEvent.ACTION_MOVE) {
                //get the amount travelled.
                dx = ((event.getX()) - previousX);
                dy = ((event.getY()) - previousY);
                previousX = event.getX();
                previousY = event.getY();
                //check for scroll event from user
                if(event.getPointerCount() > 1) {
                    //scroll up
                    if (dy < 0) {
                        scrollUp = 1;
                    }
                    //scroll down
                    else if(dy > 0) {
                        scrollDown = 1;
                    }
                }
                try {
                    DatagramPacket packet = new DatagramPacket(new byte[]{MOUSE_EVENT, (byte) dx, (byte) dy,
                            0, 0, (byte)scrollUp, (byte)scrollDown}, 7);
                    udpHandler.udpSend(packet);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return super.onTouchEvent(event);
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            setupSettingsView();
            return true;
        }
        else if(id == R.id.action_quit) {
            udpHandler.disconnect();
            this.finish();
        }

        return super.onOptionsItemSelected(item);
    }
    private void setupKeyboardButton(Button keyboardButton) {
        keyboardButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final int[] previousLength = {0};
                LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                View keyboardView = inflater.inflate(R.layout.keyboard_layout, null);
                EditText keyboardInput = (EditText) keyboardView.findViewById(R.id.KeyboardInput);
                keyboardInput.addTextChangedListener(new TextWatcher() {
                    @Override
                    public void beforeTextChanged(CharSequence s, int start, int count, int after) {

                    }

                    @Override
                    public void onTextChanged(CharSequence s, int start, int before, int count) {

                    }

                    @Override
                    public void afterTextChanged(Editable s) {
                        if(s.length() > previousLength[0] && udpHandler != null) {
                            try {
                                DatagramPacket packet = new DatagramPacket(new byte[]{KEYBOARD_EVENT,
                                        (byte) s.charAt(s.length() - 1)}, 2);
                                udpHandler.udpSend(packet);
                                previousLength[0] = s.length();
                            }catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                        else if(udpHandler != null){
                            try {
                                DatagramPacket packet = new DatagramPacket(new byte[] {KEYBOARD_EVENT ,
                                        KEY_BACKSPACE}, 2);
                                udpHandler.udpSend(packet);
                                previousLength[0] = s.length();
                            }catch (IOException e) {
                                e.printStackTrace();
                            }
                            System.err.println("Delete pressed");
                        }
                    }
                });
                setContentView(keyboardView);
                inKeyboardView = true;
            }
        });
    }
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (inKeyboardView || inSettingsView) {
            if (keyCode == KeyEvent.KEYCODE_BACK) {
                inKeyboardView = false;
                inSettingsView = false;
                setMainDisplay();
                return true;
            }
        }
        return super.onKeyUp(keyCode, event);
    }
    private void setupMouseButtons(Button left, Button right) {
        left.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    DatagramPacket packet = new DatagramPacket(new byte[]{MOUSE_EVENT, 0,
                            0, MOUSE_LEFT_CLICK, 0}, 5);
                    udpHandler.udpSend(packet);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
        right.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    DatagramPacket packet = new DatagramPacket(new byte[]{MOUSE_EVENT, 0,
                            0, 0, MOUSE_RIGHT_CLICK}, 5);
                    udpHandler.udpSend(packet);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
    }
    private void setupSettingsView() {
        //inflate the settings view
        final LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        final View settingsView = inflater.inflate(R.layout.settings_layout, null);
        //set the port number input
        final EditText portInput = (EditText) settingsView.findViewById(R.id.PortNumberInput);
        portInput.setText(Integer.toString(serverPortNumber));
        //set the IP input
        final EditText ipInput = (EditText) settingsView.findViewById(R.id.DefaultIP);
        ipInput.setText(subnetString);
        //set amount to query input
        final EditText amount = (EditText) settingsView.findViewById(R.id.MaxAddress);
        amount.setText("50");
        //set the 'save' button
        Button saveButton = (Button) settingsView.findViewById(R.id.SaveButton);
        setupSaveButton(saveButton, portInput);
        //setup the ListView for queried network peers
        final ListView listView = (ListView) settingsView.findViewById(R.id.AvailableListView);
        final ArrayAdapter arrayAdapter = new ArrayAdapter(this,R.layout.listview_row, R.id.IpStringView, availableServers);
        listView.setAdapter(arrayAdapter);
        //setup the click listener for ListView objects
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                try {
                    //set the selected IP
                    serverIp = InetAddress.getByName(availableServers.get(position));
                    //visibly select the item
                    view.setBackgroundColor(Color.CYAN);
                    //save the selection
                    if (previousListViewSelection == null) {
                        previousListViewSelection = view;
                    }
                    //'un'select the item on change
                    else {
                        previousListViewSelection.setBackgroundColor(Color.WHITE);
                        previousListViewSelection = view;
                    }
                } catch (UnknownHostException e) {
                    e.printStackTrace();
                }
            }
        });
        //setup the 'scan' button to query network peers
        final Button scanButton = (Button) settingsView.findViewById(R.id.ScanButton);
        final ProgressBar progressBar = (ProgressBar) settingsView.findViewById(R.id.progressBar);
        progressBar.setVisibility(View.INVISIBLE);
        //call to method that adds functionality to 'scan' button
        setupScanButton(scanButton, listView, progressBar, arrayAdapter, ipInput, amount);
        //display our view
        setContentView(settingsView);
        //notify we are in settings view
        inSettingsView = true;
    }

    private void setupSaveButton(Button saveButton, final EditText portInput) {
        saveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //if the port number was changed....
                if(!portInput.getText().toString().equals("6666")) {
                    //set the new port number
                    try {
                        serverPortNumber = Integer.parseInt(String.valueOf(portInput.getText()));
                    }catch(NumberFormatException ex) {
                        makeToast("Wrong port number input!", getApplicationContext());
                    }
                }
                //need a valid IP
                if(serverIp == null) {
                    makeToast("No IP selected! Try Again!", getApplicationContext());
                }
                else {
                    //open a handler for communication
                    try {
                        udpHandler = new UDPHandler(serverIp, serverPortNumber);
                        udpHandler.connect();
                        statusString = "Connected: " + serverIp.getHostName()
                                        + " " + serverPortNumber;
                    } catch (SocketException e) {
                        e.printStackTrace();
                        makeToast("Error creating connection!", getApplicationContext());
                    }
                }
                //leave the settings view.
                inSettingsView = false;
                setMainDisplay();
            }
        });
    }

    /*
        Method to add functionality to the 'Scan' button within the Settings view.
        During a scan procedure, we take the subnet string from the settings view,
        and loop through the addresses while calling .isReachable() on each address.
        This will allow us to see which address is accessible.
     */
    private void setupScanButton(final Button scanButton, final ListView listView,
                                 final ProgressBar progressBar, final ArrayAdapter arrayAdapter,
                                 final EditText ipInput, final EditText amountToQuery) {

        scanButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                subnetString = ipInput.getText().toString();
                if(listView.getCount() > 0) {
                    availableServers = new ArrayList<String>();
                    listView.setAdapter(arrayAdapter);
                    arrayAdapter.notifyDataSetChanged();
                }
                Thread discoverThread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        scanButton.post(new Runnable() {
                            @Override
                            public void run() {
                                progressBar.setVisibility(View.VISIBLE);
                            }
                        });
                        int timeout = 500;
                        for (int i = 1; i < Integer.parseInt(amountToQuery.getText().toString()); i++) {
                            String host = subnetString.concat("." + Integer.toString(i));
                            try {
                                if (InetAddress.getByName(host).isReachable(timeout)) {
                                    System.err.println(host + "is reachable");
                                    availableServers.add(host);
                                    scanButton.post(new Runnable() {
                                        @Override
                                        public void run() {
                                            arrayAdapter.notifyDataSetChanged();
                                        }
                                    });
                                }
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                        scanButton.post(new Runnable() {
                            @Override
                            public void run() {
                                progressBar.setVisibility(View.INVISIBLE);
                            }
                        });
                    }
                });
                discoverThread.start();
            }
        });
    }
    private void makeToast(String string, Context context) {
        Toast toast = Toast.makeText(context, string, Toast.LENGTH_LONG);
        toast.show();
    }
}
