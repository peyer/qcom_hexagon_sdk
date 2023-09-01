package com.example.qualcomm.calculator;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import java.io.File;

public class MainCalculator extends AppCompatActivity {

    public static final String RESULT_MESSAGE = "com.example.qualcomm.calculator.MESSAGE";
    // Used to load the 'calculator' library on application startup.
    static {
        System.loadLibrary("calculator");
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main_calculator);
        try {
            //Set the path of libcalculator_skel.so
            //Push the skel shared object to the location /data/app
            String skel_location = "/data/app";
            System.out.println("Skel library location : " + skel_location);
            //Set the ADSP_LIBRARY_PATH to skel_location
            init(skel_location);
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }
    /** Called when the user taps the Send button */
    public void sendMessage(View view) {
        Intent intent = new Intent(this, DisplayResult.class);
        EditText editText = findViewById(R.id.editText);
        String message;
        try {
            //Create an array of length len defined by user
            int len = Integer.valueOf(editText.getText().toString());
            int[] vec = new int[len];
            for (int i = 0; i < len; i++) {
                vec[i] = i;
            }
            message = "Result: The sum of " + String.valueOf(len) + " numbers is " + String.valueOf(sum(vec, len));
        } catch (Exception e) {
            message = "Please retry with a valid number !";
        }
        intent.putExtra(RESULT_MESSAGE, message);
        startActivity(intent);
    }
    /**
     * The native methods that are implemented by the 'calculator' native library,
     * which is packaged with this application.
     * init: input is Skel_location which sets ADSP_LIBRARY_PATH
     * sum: input is a vector containing data, and its length
     * return value is the sum of the whole vector
     */
    public native int init(String skel_location);
    public native long sum(int[] vec, int len);
}
