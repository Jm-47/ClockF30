include <lib/gear-lib.scad>

thickness = 2;

entraxe = 35;
desaxe = 8;

// Axe du moteur
h_bit = 6;
r_bit = 2.7;
meplat = 3.3;

//h_head = 3;
//r_head = 4.5;

// Boulon
r_shaft = 2;

// Gears
n_teeth = 20;
gear_size = 17.4;


module Text(letter, size, height, valign) {
    // convexity is needed for correct preview
    // since characters can be highly concave
    linear_extrude(height = height, convexity = 0)
        text(letter,
        size = size,
        font = "Allerta Stencil",
        //font="Monaco",                 
        halign = "center",
        valign = valign);
}

module support(height) {
    difference() {
        union() {
            cylinder(h = height, r = r_shaft + 2, $fn = 30);
        }
        union() {
            translate([0, 0, -1]) cylinder(h = height + 2, r = r_shaft, $fn = 30);
        }
    }
}

module support_double(height) {
    support(height);
    translate([0, entraxe, 0]) support(height);

    // Location of the engine axle
    if (show_engine_axle) color("white") translate([8, entraxe / 2, height - h_bit - 4]) engine_axle();
}
module engine_axle() {
    intersection() {
        cylinder(h = h_bit, r = r_bit, $fn = 30);
        translate([-meplat / 2, -r_bit - .5, 0]) cube([meplat, r_bit * 2 + 1, h_bit + 2]);
    }
}

module engine_axle_plug() {
    cylinder(h = h_bit, r = r_bit + 2, $fn = 30);
}


support = false;
gear1 = false;
gear2 = true;
axle = false;

show_engine_axle = false;
 

if (support) {
    difference() {
        union() {
            translate([-20, -10]) cube([70, 100, 2]);
            support_double(16);
            translate([40, entraxe, 0]) rotate([0, 0, 180]) support_double(14);
            translate([40 - 8, entraxe / 2, 2]) translate([0, 0, -.5]) cylinder(h = 1, r1 = 2, r2 = 0, $fn = 30);
        }
        union() {
            translate([8, -5, -1]) cylinder(h = 4, r = r_shaft, $fn = 30);
            translate([8, entraxe / 2, -1]) cylinder(h = 4, d = 5.1, $fn = 30);
            translate([0, 0, -1]) cylinder(h = 4, r = r_shaft, $fn = 30);
            translate([0, entraxe, -1]) cylinder(h = 4, r = r_shaft, $fn = 30);
            translate([40, 0, -1]) cylinder(h = 4, r = r_shaft, $fn = 30);
            translate([40, entraxe, -1]) cylinder(h = 4, r = r_shaft, $fn = 30);
            rotate([0, 0, 90]) translate([40, 10, -1]) Text("Le Fourneau a 30 ans !", 6, 4);
        }
    }
}

// First gear
if (gear1) color("red") translate([40 - 8, entraxe / 2, 2])
    difference() {
        union() {
            rotate([0, 0, 360 / (n_teeth * 2)]) gear(n_teeth, 8, gear_size);
            translate([0, 0, .5]) engine_axle_plug();
        }
        union() {
            translate([0, 0, -.5]) cylinder(h = 1, r1 = 3, r2 = 0, $fn = 30);
            translate([0, 0, 1]) engine_axle();
        }
    }

// Second gear
if (gear2) color("blue") translate([8, entraxe / 2, 2])
    difference() {
        union() {
            gear(n_teeth, 8, gear_size);
            translate([0, 0, -15]) cylinder(h = 16, d = 5, $fn = 30);
        }
        translate([0, 0, -16]) cylinder(h = 20, r = 2, $fn = 30);
    }

// Direct axle
if (axle) color("green")   translate([8, entraxe / 2, 2])
    difference() {
        union() {
            translate([0, 0, 2]) engine_axle_plug();
            translate([0, 0, -24]) cylinder(h = 30, r = 1.8, $fn = 30);
        }
        translate([0, 0, 3.5]) engine_axle();
    }
