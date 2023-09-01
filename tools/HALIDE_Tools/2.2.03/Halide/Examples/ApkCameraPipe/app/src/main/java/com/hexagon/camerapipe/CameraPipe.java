package com.hexagon.camerapipe;

import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public class CameraPipe extends AppCompatActivity {

    String output = null;
    String input = "/storage/emulated/0/bayer_raw_input.pgm";
    String color_temp = "3700";
    String gamma = "2.0";
    String contrast = "50";
    String timing_iteration = "5";
    double time_taken = 0;
    ImageView iv = null, iv1 = null;
    TextView time_taken_text = null, time_heading = null, status_message = null;
    static Bitmap input_bmp = null;
    File sdCardDir = Environment.getExternalStorageDirectory();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        output = getFilesDir().getAbsolutePath() + "/out_bayer_raw.ppm";
        setContentView(R.layout.activity_hello_hexagon);
        iv = (ImageView)findViewById(R.id.iv);
        iv1 = (ImageView)findViewById(R.id.iv1);
        time_taken_text = (TextView)findViewById(R.id.timeText);
        time_heading = (TextView)findViewById(R.id.timeHead);
        status_message = (TextView)findViewById(R.id.statusMessage);
        Log.d("Camera Pipe Class", "SD card location: " + sdCardDir.getAbsolutePath());

        // Check for file permissions

        iv.setVisibility(View.VISIBLE);
        iv1.setVisibility(View.VISIBLE);
        status_message.setText("Displaying the input image. Please wait ...");
        //Log.d("Camera Pipe Class", "Setting input bitmap for " + input);
        //bitmap= BitmapFactory.decodeResource(getResources(),R.drawable.snapdragon);//reading the image from drawable
        ActivityCompat.requestPermissions(this, new String[]{android.Manifest.permission.READ_EXTERNAL_STORAGE}, 1);

        Button hvxButton = (Button) findViewById(R.id.hvxButton);
        hvxButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v){
                new processImageTask().execute(input, color_temp, gamma, contrast, timing_iteration, output, "hvx");
                new ReadBitmapFromPPM2Task().execute(output, "1"); // 1 for output file
            }
        });
        Button cpuButton = (Button) findViewById(R.id.cpuButton);
        cpuButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v){
                new processImageTask().execute(input, color_temp, gamma, contrast, timing_iteration, output, "cpu");
                new ReadBitmapFromPPM2Task().execute(output, "1"); // 1 for output file
            }
        });
        Button resetButton = (Button) findViewById(R.id.resetButton);
        resetButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v){
                //Log.d("Camera Pipe Class", "Resetting image");
                iv.setImageResource(0);
                iv.setImageBitmap(input_bmp);// display the decoded image
                iv1.setImageResource(0);
                time_heading.setText("");
                time_taken_text.setText("");
                status_message.setText("");
            }
        });



    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        if(requestCode == 1){
            if (grantResults.length > 0
                    && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                new ReadBitmapFromPGMTask().execute(input, "0"); // 0 for input file
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_hello_hexagon, menu);
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
            return true;
        }

        return super.onOptionsItemSelected(item);
    }



    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native double processImageOnHVX(String input, String color_temp, String gamma, String contrast, String timing_iteration, String output, String CpuOrHVX);
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private class processImageTask extends AsyncTask<String, Void, Double> {
        String starget = null;
        @Override
        protected void onPreExecute() {
            status_message.setText("Processing Image. Please wait ...");
        }
        @Override
        protected Double doInBackground(String... params) {
            String sinput = params[0];
            String scolor_temp = params[1];
            String sgamma = params[2];
            String scontrast = params[3];
            String stiming_iteration = params[4];
            String soutput = params[5];
            starget = params[6];
            return processImageOnHVX(sinput, scolor_temp, sgamma, scontrast, stiming_iteration, soutput, starget);
        }
        @Override
        protected void onProgressUpdate(Void... progress) {
            status_message.setText("Processing Image. Please wait ...");
        }
        @Override
        protected void onPostExecute(Double result) {
            status_message.setText("Completed the processing. Displaying the image. Please wait ...");
            time_heading.setText("Time Taken for processing on " + starget + " :");
            time_taken_text.setText(Double.toString(result) +"us");
        }
    }


    private class ReadBitmapFromPPM2Task extends AsyncTask<String, Void, Bitmap> {

        String file_type = null;
        @Override
        protected Bitmap doInBackground(String... params){
            Bitmap bmp = null;
            String file = params[0];
            file_type = params[1];
            Bitmap image = null;
            byte ibuffer[] = null;
            int obuffer[] = null;
            int i = 0;
            String NEWLINE = "\n";
            int MAX_METADATA_LEN = 40;
            int TYPE = 0;
            int DIMENSIONS = 1;
            int PIXELSIZE = 2;

            try {
                BufferedInputStream is = new BufferedInputStream(new FileInputStream(file));
                if (is.available() < MAX_METADATA_LEN) {
                    throw new IOException("Invalid file passed!!!");
                }

                byte metabuffer[] = new byte[MAX_METADATA_LEN];
                is.read(metabuffer, 0, MAX_METADATA_LEN);
                String str = new String(metabuffer, StandardCharsets.UTF_8);
                String strArray[] = str.split(NEWLINE);

                String type = strArray[TYPE];
                String dimentions = strArray[DIMENSIONS];
                String pixelSize = strArray[PIXELSIZE];

                type = type.trim();
                dimentions = dimentions.trim();
                pixelSize = pixelSize.trim();

                is.close();
                is = new BufferedInputStream(new FileInputStream(file));
                is.read(metabuffer, 0, type.length() + dimentions.length() + pixelSize.length() + 3);

                if (0 == type.compareTo("P6")) {
                    if (!dimentions.isEmpty() && !pixelSize.isEmpty()) {
                        int size = Integer.parseInt(pixelSize);
                        String[] dim = dimentions.split(" ");
                        if (dim.length == 2 && size == 255) {
                            int width = Integer.parseInt(dim[0]);
                            int height = Integer.parseInt(dim[1]);

                            ibuffer = new byte[3];
                            obuffer = new int[width * height];
                            for (int j = 0; j < width * height; j++) {
                                is.read(ibuffer, 0, ibuffer.length);
                                obuffer[i++] = Color.argb(0xFF, (int)(ibuffer[0]&0xFF), (int)(ibuffer[1]&0xFF), (int)(ibuffer[2]&0xFF));
                            }
                            image = Bitmap.createBitmap(obuffer, width, height, Bitmap.Config.ARGB_8888);;
                        } else {
                            Log.d("Camera Pipe Class","Invalid/Corrupt file " + file + "!!!\n");
                        }
                    } else {
                        Log.d("Camera Pipe Class","Invalid/Corrupt file " + file + "!!!\n");
                    }
                } else {
                    Log.d("Camera Pipe Class","Invalid/Corrupt file " + file + "!!!\n");
                }
            }catch (IOException e) {
                    e.printStackTrace();
                }
                return image;
        }

            @Override
        protected void onProgressUpdate(Void... progress) {
            status_message.setText("Completed the processing. Displaying the image ...");
        }
        @Override
        protected void onPostExecute(Bitmap result) {
            if(file_type.equals("0")) {
                input_bmp = result; // use for resetting
                iv.setImageBitmap(result);// display the decoded image
                status_message.setText("");
            }
            else {
                iv1.setImageBitmap(result);// display the decoded image
                status_message.setText("");
            }
        }

    }

    private class ReadBitmapFromPGMTask extends AsyncTask<String, Void, Bitmap> {

        String file_type = null;

        @Override

        protected Bitmap doInBackground(String... params){
            Bitmap bmp = null;
            String file = params[0];
            file_type = params[1];
            Bitmap image = null;
            byte ibuffer[] = null;
            int obuffer[] = null;
            int i = 0;
            String NEWLINE = "\n";
            int MAX_METADATA_LEN = 40;
            int TYPE = 0;
            int DIMENSIONS = 1;
            int PIXELSIZE = 2;


            try {
                BufferedInputStream is = new BufferedInputStream(new FileInputStream(file));
                if (is.available() < MAX_METADATA_LEN) {
                    throw new IOException("Invalid file passed!!!");
                }

                byte metabuffer[] = new byte[MAX_METADATA_LEN];
                is.read(metabuffer, 0, MAX_METADATA_LEN);
                String str = new String(metabuffer, StandardCharsets.UTF_8);
                String strArray[] = str.split(NEWLINE);

                String type = strArray[TYPE];
                String dimentions = strArray[DIMENSIONS];
                String pixelSize = strArray[PIXELSIZE];

                type = type.trim();
                dimentions = dimentions.trim();
                pixelSize = pixelSize.trim();

                is.close();
                is = new BufferedInputStream(new FileInputStream(file));
                is.read(metabuffer, 0, type.length() + dimentions.length() + pixelSize.length() + 3);


                if(0 == type.compareTo("P5")) {
                    if(!dimentions.isEmpty() && !pixelSize.isEmpty()) {
                        int size = Integer.parseInt(pixelSize);
                        String[] dim = dimentions.split(" ");
                        if(dim.length == 2 && size == 65535) {
                            int width = Integer.parseInt(dim[0]);
                            int height = Integer.parseInt(dim[1]);

                            ibuffer = new byte[2];
                            obuffer = new int[width * height];
                            for (int j = 0, index = 0; j < width * height; j++, index++) {
                                is.read(ibuffer, 0, ibuffer.length);
                                int data = (ibuffer[0] & 0xFF);
                                data = data << 8;
                                data = data | (ibuffer[1] & 0xFF);
                                data = data >> 2;
                                obuffer[i++] = Color.argb(0xFF, data, data, data);
                            }
                            image = Bitmap.createBitmap(obuffer, width, height, Bitmap.Config.ARGB_8888);
                        } else {
                            Log.d("Camera Pipe Class","Invalid/Corrupt file " + file + "!!!\n");
                        }
                    } else {
                        Log.d("Camera Pipe Class","Invalid/Corrupt file " + file + "!!!\n");
                    }
                } else {
                    Log.d("Camera Pipe Class","Invalid/Corrupt file " + file + "!!!\n");
                }
            }catch (IOException e) {
                e.printStackTrace();
            }
            return image;
        }

        @Override
        protected void onProgressUpdate(Void... progress) {
            status_message.setText("Completed the processing. Displaying the image ...");
        }
        @Override
        protected void onPostExecute(Bitmap result) {
            if(file_type.equals("0")) {
                input_bmp = result; // use for resetting
                iv.setImageBitmap(result);// display the decoded image
                status_message.setText("");
            }
            else {
                iv1.setImageBitmap(result);// display the decoded image
                status_message.setText("");
            }
        }
    }

}
