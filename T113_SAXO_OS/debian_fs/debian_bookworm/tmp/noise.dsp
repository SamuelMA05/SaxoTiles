declare options "[osc:on]";
import("stdfaust.lib");
process = no.noise*hslider("level",0.02,0,1,0.01);
