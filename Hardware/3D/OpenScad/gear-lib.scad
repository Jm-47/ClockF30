
/**************** GEARS ********************
gear(nb-teeth,thikness,scale); // min 10
********************************************/

module gr_1(n=0,thk=0){

    r = n*1.5;
  
module tooth(){
    sz = 5;
    sx = 3;
    th = thk;
    of = 5;
        
    hull(){
    translate([0,0,th/2])
    cube([sz,sz,th],center=true); 
    translate([of,0,th/2])
    cube([sx,sx,th],center=true);}}

    for(i=[0:n]) //n is number of teeth
    rotate([0,0,i*360/n])
    translate([r,0,0]) 
    tooth();
    
    cylinder(r=r,h=thk);}

module gear(n=0,thk=0,sc=0){
    sc = sc*0.0204;
    scale(sc)
    gr_1(n=n,thk=thk);}


/********** WORM **********/
    
module grx(nn=0,thkk=0){
  
module tootx(){
    sz = 5;
    sx = 3;
    th = thkk-1.8;
    of = 5;
        rotate([90,0,0])
    hull(){
    translate([0,0,th/2])
    cube([sz,sz,th],center=true); 
    translate([of,0,th/2])
    cube([sx,sx,th],center=true);}}

    for (n = [1 : 450]){
    rotate([0,0,n*4]){
        translate([10,0,n*0.12])
        tootx();
    }}
   // color("yellow")
    translate([0,0,-2.4])
    cylinder(d=16,h=59);
    }
    
module worm(scl=0){
    nn=5;
    thkk=3;
    scl = scl*0.0204;
    scale(scl)
    grx(nn=nn,thkk=thkk);}
    

/**************** GEARS *******************/
