// Copyright 2021 CDO Apps
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.cdoapps.sample;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.DataBindingUtil;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.os.Bundle;
import android.view.ViewGroup;

import com.cdoapps.gpio.Thermometer;
import com.cdoapps.gpio.GPIO;
import com.cdoapps.gpio.OneWire;
import com.cdoapps.sample.databinding.ActivityMainBinding;
import com.cdoapps.sample.databinding.ActivityMainItemBinding;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MainActivity extends AppCompatActivity {
    static class ItemViewHolder extends RecyclerView.ViewHolder {
        public ItemViewHolder(ActivityMainItemBinding binding) {
            super(binding.getRoot());

            this.binding = binding;
        }

        ActivityMainItemBinding binding;

        public void configure(Map<String, String> item) {
            binding.romTextView.setText(item.get("rom"));
            binding.familyTextView.setText(item.get("family"));
            binding.temperatureTextView.setText(item.get("temperature"));
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = DataBindingUtil.setContentView(this, R.layout.activity_main);
        thread = null;
        items = new ArrayList<>();

        binding.recyclerView.setLayoutManager(new LinearLayoutManager(getApplicationContext()));
        binding.recyclerView.setAdapter(new RecyclerView.Adapter() {
            @Override
            public int getItemViewType(int position) {
                return R.layout.activity_main_item;
            }

            @NonNull
            @Override
            public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
                return new ItemViewHolder(DataBindingUtil.inflate(getLayoutInflater(), viewType, parent, false));
            }

            @Override
            public void onBindViewHolder(@NonNull RecyclerView.ViewHolder viewHolder, int position) {
                ItemViewHolder holder = (ItemViewHolder) viewHolder;
                holder.configure(items.get(position));
            }

            @Override
            public int getItemCount() {
                return items.size();
            }
        });
    }

    ActivityMainBinding binding;
    Thread thread;
    List<Map<String, String>> items;

    @Override
    protected void onResume() {
        super.onResume();

        GPIO gpio = GPIO.getInstance();
        gpio.onResume();

        OneWire oneWire = new OneWire();
        oneWire.configureBuffered(gpio, 0, 2);

        thread = new Thread() {
            @Override
            public void run() {
                List<Thermometer> thermometers = Thermometer.listAll(oneWire);

                boolean stop = false;
                while (!stop) {
                    Thermometer.convert(oneWire, thermometers);

                    synchronized (items) {
                        items.clear();
                        for (Thermometer thermometer : thermometers) {
                            Map<String, String> item = new HashMap<>();
                            item.put("rom", thermometer.getRom());
                            item.put("family", thermometer.getFamily().toString());
                            item.put("temperature", thermometer.getTemperature() + "°C");
                            items.add(item);
                        }
                    }

                    binding.getRoot().post(() -> {
                        synchronized (items) {
                            binding.recyclerView.getAdapter().notifyDataSetChanged();
                        }
                    });

                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        stop = true;
                    }
                }

                Thermometer.destroyAll(thermometers);
                oneWire.destroy();
            }
        };

        thread.start();
    }

    @Override
    protected void onPause() {
        super.onPause();

        thread.interrupt();
        try {
            thread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        thread = null;

        GPIO gpio = GPIO.getInstance();
        gpio.unexportAll();
        gpio.onPause();
    }
}