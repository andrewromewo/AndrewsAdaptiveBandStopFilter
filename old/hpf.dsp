import("stdfaust.lib");

FC = hslider("Freq", 1000, 20, 12000, 1): si.smoo;

filter_hpf = fi.highpass(6,FC);
filter_lpf = fi.lowpass(2,8000);

process = _,_: filter_lpf,filter_lpf : _,_;

