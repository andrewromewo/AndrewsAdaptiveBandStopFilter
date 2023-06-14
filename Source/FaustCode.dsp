import("stdfaust.lib");

FC = hslider("Freq", 1000, 20, 20000, 1): si.smoo;
wetness = hslider("WetDry", 0, 0, 1, 0.01);


filter_hpf = fi.highpass(6,FC+FC*0.2);
filter_lpf = fi.lowpass(6,FC-FC*0.2);

bandstop = _<:filter_hpf , filter_lpf:>_;
bandstop_mixed = _ <: bandstop*wetness,_*(1-wetness):> _;

process = _,_: bandstop_mixed, bandstop_mixed: _,_;
