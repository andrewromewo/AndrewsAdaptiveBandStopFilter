import("stdfaust.lib");
process = _,_ : re.greyhole(4,0.1,2,0.2,0.1,0.1,2): _,_;
// process = _,_;
// process = _,_ : fi.pole(0.7),fi.pole(0.7) :_,_;
