/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2014                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
/*  FUNCTIONS      : Shuffle Exchange Control Generator                 */
/*  ARCHITECTURE   :  any                                               */
/*  VERSION        : 2.1                                                */
/*                                                                      */
/*======================================================================*/
/*  REVISION HISTORY:                                                   */
/*  =================                                                   */
/*                                                                      */
/*  Author       Date       Comments                                    */
/*  -------------------------------------------------------------       */
/*  DJH          02/09/11   created iniitial Benese Netowrk Generator   */
/*  DJH          07/06/11   modified for shuffle Exchange INsturction   */
/*  DJH          08/26/11   reversed control bits, changed sense of the */
/*                          permute understandign input by the algorithm*/
/*  DJH          06/06/13   modified for vrdelta and vdelta networks    */
/*  JW           02/21/14   javascript'ed                               */
/*======================================================================*/

//Javascript Helpers
function objectarray(array) { for(index=0; index<array.length; index++) { array[index] = {}; } }
function arrayarray(array,size) { for(index=0; index<array.length; index++) { array[index] = new Array(size); } }
function printhex(value) {
    if (value.toString(16).length == 1) {
	return "0"+value.toString(16).toUpperCase();
    }
    if (value.toString(16).length == 2) {
	return value.toString(16).toUpperCase();
    }
}

function set_logN6() {
    logN.value = 6;
    tperm.value = "0 ,2 ,3 ,4 ,6 ,7 ,8 ,9 ,11,12,13,14,16,17,18,19,21,22,23,24,26,27,28,29,31,32,33,35,36,37,38,40,41,42,43,45,46,47,48,50,51,52,53,55,56,57,58,60,61,62,1 ,5 ,10,15,20,25,30,34,39,44,49,54,59,63";
    tpermx.value = "1,2,3,5,6,7,8,10,11,12,13,15,16,17,18,20,21,22,23,25,26,27,28,30,31,32,34,35,36,37,39,40,41,42,44,45,46,47,49,50,51,52,54,55,56,57,59,60,61,62,X,X,X,X,X,X,X,X,X,X,X,X,X,X";

}

function set_logN7() {
    logN.value = 7;
    tperm.value = "0 ,2 ,3 ,4 ,6 ,7 ,8 ,9 ,11,12,13,14,16,17,18,19,21,22,23,24,26,27,28,29,31,32,33,35,36,37,38,40,41,42,43,45,46,47,48,50,51,52,53,55,56,57,58,60,61,62,1 ,5 ,10,15,20,25,30,34,39,44,49,54,59,63,64,66,67,68,69,71,72,73,74,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,65,70,75";
    tpermx.value = "1,2,3,5,6,7,8,10,11,12,13,15,16,17,18,20,21,22,23,25,26,27,28,30,31,32,34,35,36,37,39,40,41,42,44,45,46,47,49,50,51,52,54,55,56,57,59,60,61,62,64,65,66,67,69,70,71,72,73,74,76,77,78,79,80,81,82,84,85,86,87,88,90,91,92,93,94,95,96,98,99,101,102,103,104,106,107,108,109,111,112,114,115,118,119,120,123,125,126,127,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X";
}

set_logN6();

var outdiv = document.getElementById('ccode');

function outer_function(runme,arg) {

//console.log(runme);
//console.log(arg);

var logN = parseInt(document.getElementById('logN').value);
//console.log(logN);
var MAX_TESTS=1;
var N=1<<logN;
var DEBUG=0;
var DISPLAY=1;
var REV_BUTTERFLY=1;

var VWIDTH=N;
var X=-1;

var gtemp = new Array(2*N);

var gsc = new Array(N);
arrayarray(gsc,4*logN);

var gsb = new Array(N);
arrayarray(gsb,4*logN);

var gss = new Array(N);
arrayarray(gss,4*logN);

var group = new Array(N*2) //group of 4 x n/2 element toppath is even bot i odd
var vperm = new Array(N);
var block = 0;

/*======================================================================*/
function printint(a) {
    if(a < 10)
	outdiv.innerHTML+=a+" ";
    else if(a < 100)
	outdiv.innerHTML+=a+"";
    else
	outdiv.innerHTML+=a+"";
}

function brev(x, order) {
    var y = 0;
    var i;
    var mask = 0xffffffff >>> (32-order) ;
    var base = x & ~mask;

    if(!x) return(0);
    x = x & mask;
    for(i=0; i < order; i++) {
	y <<= 1;
	y |= x & 1;
	x >>= 1;
    }
    return( base | y);
}

function deal2(x, s) {
    var z;
    var y;
    var mask;
    var i;
    mask  = (1<<s)-1;
    z = x & ~mask;
    x = x & mask;
    y = 1& (x>>(s-1));
    y = z |y | ((x<<1) & mask);
    return(y);
}

function deal4(x, s) {
    var y = 0;
    var mask = (1<<s)-1;
    y = (x)>>(s-2);
    y = y | (x<<2);
    return(y& mask);
}

function shfl2(x, s) {
    var y;
    var z;
    var mask = (1<<s)-1;
    z = x & ~mask;
    x = x & mask;
    y = (x & 1)<<(s-1);
    y = z | y | (x>>1);
    return(y);
}

function shfl4(x, s) {
    var y = 0;
    var mask = (1<<s)-1;
    y = (x & 3)<<(s-2);
    y = y | (x>>2);
    return(y& mask);
}

function rand_perm(perm, n){
    var i, j, k, tmp;

    for(i=0; i < n; i++)
    {
	perm[i] = i;
    }
    for(k=0; k < 32111; k++)
    {
	i = Math.floor(Math.random()*n);
	i ^= 0x5A5A5 % n;
	j = Math.floor(Math.random()*n);
	j ^= 0xA5A5A % n;
	tmp = perm[i];
	perm[i] = perm[j];
	perm[j] = tmp;
    }
}

function collect_switches(n, logn)
{
    var i, j;
    if (DEBUG) {
	outdiv.innerHTML+=" n logn "+n+" "+logn+"<br>";
	for(i=0; i < Math.floor(n/2); i++)
	{
	    for(j=0; j < 2*logn; j++) outdiv.innerHTML+=gsb[i][j]+", ";
	    outdiv.innerHTML+="<br>";
	}
    }

    for(i=0; i < Math.floor(n/2); i++)
	gsb[i][logn-1] ^= gsb[i][logn];
    for(j=logn; j < 2*logn-1; j++)
	for(i=0; i < Math.floor(n/2); i++) {
	    gsb[i][j] = gsb[i][j+1];
	}
}

function orthogonalizen(toppath, bot, inperm, outperm, revperm, present, n)
{
    var i,j,k,l, bc, tc;
    var ifx, clean, done, trys;
    var row, start;
    var sib, entry, swap, node;

    if (DEBUG) {
	console.log("map ");
	for(i=0; i < n; i++) console.log(inperm[i]+"->"+outperm[i]);
	console.log("reverse map ");
	for(i=0; i < n; i++) console.log(inperm[i]+"->"+revperm[i]);
    }

    for(i=0; i < 2*n; i++) group[i]=-1;
    for(start = 0; start < n; start+=2) {
	node = start;
	row = Math.floor(node/2);
	if(group[4*row] == -1) {
	    group[4*row  ] = inperm[node];   group[4*row+1] = outperm[node];
	    group[4*row+2] = inperm[node+1]; group[4*row+3] = outperm[node+1];
	    sib = revperm[outperm[node]^1];
	    while(sib != (start+1)) {
		row = Math.floor(sib/2);
		group[4*row  ] = inperm[sib^1];
		group[4*row+1] = outperm[sib^1];
		group[4*row+2] = inperm[sib];
		group[4*row+3] = outperm[sib];
		node = sib^1;
		sib = revperm[outperm[node]^1];
	    }
	}//end if
    }//end start

    for(i=0; i < Math.floor(n/2); i++) {
	toppath[i].input = group[4*i];
	toppath[i].out= group[4*i+1];
	present[group[4*i  ]]=0;
	bot[i].input = group[4*i+2];
	bot[i].out= group[4*i+3];
	present[group[4*i+2]]=1;
    }
}

function gen_switch_cntrl(toppath, bot, n, inperm, outperm, insb, outsb, column, row)
{
    var i, j,tc, bc, tmp;
    var inpermh = new Array (N);
    var inperml = new Array (N);
    var outpermh = new Array (N);
    var outperml = new Array (N);
    var outvec = new Array (N);
    var present = new Array (N);

    if(n == 1) {return(0);}
    for(i=0; i < n; i++) outvec[outperm[i]] = inperm[i];
    if(n==32) {
	for(i=0; i < 32; i++) {
            vperm[i+32*block] = brev(outvec[brev(i,5)], 5);
	}
	block++;
    }
    if(n > 1) orthogonalizen(toppath, bot, inperm, outperm, outvec, present, n);
    for(i=0; i < Math.floor(n/2); i++)
    {
	toppath[i].dst = outvec[toppath[i].input];
	bot[i].dst = outvec[bot[i].input];
	if(inperm[2*i]== toppath[i].input) insb[i] = 0;  else insb[i] = 1;
	outsb[i] = present[outvec[2*i]];
	gsb[row+i][column] = insb[i];
	gsb[row+i][2*logN-1-column] = outsb[i];
    }
    for(i=0; i < Math.floor(n/2); i++)
    {
	//initial output values
	outvec[2*i+1] >>= 1;
	outvec[2*i] >>= 1;
	tmp = outvec[2*i+1];
	if(outsb[i]) {
            outvec[2*i+1] = outvec[2*i];
            outvec[2*i] = tmp;
	}
	//now the path controls
	toppath[i].input >>= 1;
	bot[i].input >>= 1;
	toppath[outvec[2*i]].out  = toppath[i].input ;
	bot[outvec[2*i+1]].out = bot[i].input ;
    }
    for(i=0; i < Math.floor(n/2); i++)
    {
	inpermh[i] = toppath[i].input; outpermh[i] = toppath[i].out;
	inperml[i] = bot[i].input; outperml[i] = bot[i].out;
    }
    if (DEBUG) { console.log("calling switch size "+Math.floor(n/2)); }
    gen_switch_cntrl(toppath, bot, Math.floor(n/2), inpermh, outpermh, insb, outsb, column+1, row);
    if (DEBUG) { console.log("second switch "); }
    gen_switch_cntrl(toppath, bot, Math.floor(n/2), inperml, outperml, insb, outsb, column+1, row+n/4);
    return(0);
}

function convert_const_geo() {
    var temp = new Array(N);
    var i,j,k,l,m,n, s;

    m = Math.floor(N/2);
    for(n=2,s=2, k=4; n < logN; n++,k<<=1,s++) {
        for(i=0; i < k; i++) {
            for(j=0; j < Math.floor(m/k); j++) {
                temp[Math.floor(i*m/k) + j] = gsb[Math.floor(m/k)*brev(i,s) + j][n];
            }
        }
        for(i=0; i < m; i++) gsb[i][n] = temp[i];
    }
    k>>=2; s-=2;
    for(n=logN; n < 2*logN-3; n++,k>>=1,s--) {
        for(i=0; i < k; i++) {
            for(j=0; j < Math.floor(m/k); j++) {
                temp[Math.floor(i*m/k) + j] = gsb[Math.floor(m/k)*brev(i,s) + j][n];
            }
        }
        for(i=0; i < m; i++) gsb[i][n] = temp[i];
    }
}

function convert_rev_butterfly()
{
    var temp = new Array(N);
    var i,j,k,l,m,n, s;
    var lo, hi;
    var logN2 = logN-1;

    lo = (1<<(logN2))-1;
    hi = 0;
    for(j=0; j < logN; j++) {
        for(i=0; i < Math.floor(N/2); i++) {
            m =  brev(lo & i, logN2-j);
            m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j);
            temp[m] = gsb[i][j];
        }
        lo >>= 1;
        hi = (hi<<1)|1;
        for(i=0; i < Math.floor(N/2); i++) gsc[i][j] = temp[brev(i,logN2)];
    }
    lo = (1<<(logN2))-1;
    hi = 0;
    for(j=0; j < logN-1; j++) {
        for(i=0; i < Math.floor(N/2); i++) {
            m =  brev(lo & i, logN2-j);
            m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j);
            temp[m] = gsb[i][2*logN-2-j];
        }
        for(i=0; i < Math.floor(N/2); i++) gsc[i][2*logN2-j] = temp[brev(i,logN2)];
        lo >>= 1;
        hi = (hi<<1)|1;
    }
}

function convert_butterfly()
{
    var temp = new Array(N);
    var i,j,k,l,m,n, s;
    var lo, hi;
    var logN2 = logN-1;
    lo = (1<<(logN2))-1;
    hi = 0;
    for(j=0; j < logN; j++) {
        for(i=0; i < Math.floor(N/2); i++) {
            m =  brev(lo & i, logN2-j);
            m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j);
            temp[m] = gsb[i][j];
        }
        lo >>= 1;
        hi = (hi<<1)|1;
        for(i=0; i < Math.floor(N/2); i++) gsc[i][j] = temp[i];
    }
    lo = (1<<(logN2))-1;
    hi = 0;
    for(j=0; j < logN-1; j++) {
        for(i=0; i < Math.floor(N/2); i++) {
            m =  brev(lo & i, logN2-j);
            m |= brev(hi & (i>>(logN2-j)),j)<<(logN2-j);
            temp[m] = gsb[i][2*logN-2-j];
        }
        for(i=0; i < Math.floor(N/2); i++) gsc[i][2*logN2-j] = temp[i];
        lo >>= 1;
        hi = (hi<<1)|1;
    }
}

function invert_permute(outperm)
{
    var i;
    for(i=0; i < N; i++) {
	gtemp[outperm[i]] = i;
    }
    for(i=0; i < N; i++) {
	outperm[i] = gtemp[i];
    }
}

function vrdelta(Vd, Vu, Vv)
{
   var offset, k;
      for (offset=1; offset<VWIDTH; offset<<=1){
        for (k = 0; k<VWIDTH; k++) {
            Vd[k] = (Vv[k]&offset) ? Vu[k^offset] : Vu[k];
        }
        for (k = 0; k<VWIDTH; k++) {
            Vu[k] = Vd[k];
        }
      }
}

function print_hvx_bits() {
    var stride;
    var Vv = new Array(2*N);
    var i,j,k,l;

    for(k=0; k < 2*N; k++) Vv[k] = 0x0;
    stride = 1;
    for(k=0; k < logN; k++) {
        for(l=0,j=0; j < N; j+=2*stride) {
            for(i=0; i < stride; i++) {
		Vv[j+i]        |= gsc[l][k]<<k;
		Vv[j+i+stride] |= gsc[l][k]<<k; l++;
            }
        }
        stride <<= 1;
    }
    stride >>= 2;
    for(k=logN-2; k >= 0; k--) {
        for(l=0,j=0; j < N; j+=2*stride) {
            for(i=0; i < stride; i++) {
		Vv[N+j+i       ] |= gsc[l][2*logN-2-k]<<k; ;
		Vv[N+stride+j+i] |= gsc[l][2*logN-2-k]<<k; l++;
            }
        }
        stride >>= 1;
    }
    outdiv.innerHTML+="<pre>";
    outdiv.innerHTML+="unsigned char vrdelta_controls[] __attribute__((aligned(64))) = {<br>";
    for(i=0; i < N; i+=16){
        for(j=0; j < 16; j++) {
            outdiv.innerHTML+="0x"+printhex(Vv[j+i])+",";
        }
	outdiv.innerHTML+="<br>";
    }
    outdiv.innerHTML+="}<br>";
    outdiv.innerHTML+="unsigned char vdelta_controls[] __attribute__((aligned(64))) = {<br>";
    for(i=N; i < 2*N; i+=16){
        for(j=0; j < 16; j++) {
            outdiv.innerHTML+="0x"+printhex(Vv[j+i])+",";
        }
	outdiv.innerHTML+="<br>";
    }
    outdiv.innerHTML+="}<br>";
    outdiv.innerHTML+="</pre>";
}

function print_control_bits() {
    var temp, i, j, k;
    for(k=0; k < logN; k++) {
        for(j=0; j < Math.floor(N/2); j+=32) {
            temp = 0;
            for(i=0; i < 32; i++) {
		temp |= gsc[j+i][k]<<i;
            }
            outdiv.innerHTML+="0x"+temp.toString(16)+",  //level "+k+" control <br>";
        }
    }
    for(k=logN; k < 2*logN-1; k++) {
        for(j=0; j < Math.floor(N/2); j+=32) {
            temp = 0;
            for(i=0; i < 32; i++) {
		temp |= gsc[j+i][k]<<i;
            }
            outdiv.innerHTML+="0x"+temp.toString(16)+",  //level "+k+" control <br>";
        }
    }
    outdiv.innerHTML+="<p>";
}

function print_bfly_flow(){
    var i, j;
    outdiv.innerHTML+="<h3>Butterfly Controls</h3>";
    for(i=0; i < Math.floor(N/2); i++)
    {
	for(j=0; j < 2*logN-1; j++)
            if(gsc[i][j]) outdiv.innerHTML+="X  "; else outdiv.innerHTML+="=  ";
	outdiv.innerHTML+="<br>";
    }
}

function print_geo_flow(){
    var i, j;
    outdiv.innerHTML+="<h3>Const GEO Controls</h3>";
    for(i=0; i < Math.floor(N/2); i++)
    {
	for(j=0; j < 2*logN-1; j++) {
	    if(gsb[i][j]) outdiv.innerHTML+="X  "; else outdiv.innerHTML+="=  ";
	}
	outdiv.innerHTML+="<br>";
    }
}

/*======================================================================*/
/* GLOBALS */
/*======================================================================*/
var mux = new Array(2*N);
var tmux = new Array(2*N);
var toppath = new Array(2*N); //path
objectarray(toppath);
var bot = new Array(2*N); //path
objectarray(bot);
var insb = new Array(2*N);
var outsb = new Array(2*N);
var inperm = new Array(2*N);
var result = new Array(2*N);

var hperm = new Array(N);
var outperm = new Array(N);
var goldperm = new Array(N);
/*======================================================================*/

function simpleperm(tperm_input)
{
    var i,j;
    var iput = new Array(N);
    var ctrl = new Array(N);
    var pos,vec;
    var oput = new Array(N);
    
    outdiv.innerHTML="<pre>"

    var tperm = tperm_input.split(',');
    for (a in tperm ) {
	tperm[a] = parseInt(tperm[a], 10);
    }

    outdiv.innerHTML+="//desired perm<br>//";
    for(i=0; i < N; i++)
    {
	if(isNaN(tperm[i])) {
           outdiv.innerHTML+="X,";
           tperm[i] = X;
        } else {
           outdiv.innerHTML+=tperm[i]+",";
        }
	iput[i] = i;
	ctrl[i] = 0;  //clear all muxes to apss through
    }
    outdiv.innerHTML+="<br>";

    for(i=0; i < N; i++)
    {
	vec = tperm[i];
	pos = i;
	if(vec != X)for(j=Math.floor(N/2); j > 0; j>>=1)
	{
            var k;
            k = (pos & j) ^ (vec & j) ;
            ctrl[pos] |= k;
            pos ^= k;
	}
    }
    vrdelta(oput, iput, ctrl);

    //outdiv.innerHTML+="//actual outputs <br>//";
    for(i=0; i < N; i++)
    {
        if(oput[i] != tperm[i] && tperm[i] != X) {
	    outdiv.innerHTML+="pattern not possible using only vrdelta<br>";
            return(0);
	}
    }
    //for(i=0; i < N; i++)
    //{
	//if(tperm[i] != X)
	//   outdiv.innerHTML+=oput[i]+",";
        //else
	//   outdiv.innerHTML+="X,";
    //}
    outdiv.innerHTML+="<br>";
    outdiv.innerHTML+="unsigned char vrdelta_controls[] __attribute__((aligned(64))) = {<br>";
    for(i=0; i < N; i+=16)
    {
        for(j=0;j<16;j++) outdiv.innerHTML+="0x"+printhex(ctrl[i+j])+",";
	outdiv.innerHTML+="<br>";
    }
    outdiv.innerHTML+="}<br>";

    return(0);
}


function main(tperm_input)
{
    var i,j,k,l,ki, test,error, bc, tc, a2, ri, row, x, y, z, d, perm, stride;
    var src, TEST;
    var in128 = new Array(128);
    var out128 = new Array(128);

    outdiv.innerHTML="<pre>"

    var tperm = tperm_input.split(',');
    for (a in tperm ) {
	tperm[a] = parseInt(tperm[a], 10);
    }

    for(TEST=0; TEST < MAX_TESTS; TEST++) {
	for(x=0;x < N; x++) inperm[x] = x;
	if (DISPLAY) {
	    outdiv.innerHTML+="//desired perm<br>//";
	    for(i=0; i < N; i++) { printint(tperm[i]); outdiv.innerHTML+=","; } outdiv.innerHTML+="<br>";
	}
	for(i=0; i < N; i++) hperm[i] = 0;
	for(i=0; i < N; i++) hperm[tperm[i]] += 1;
	for(i=0; i < N; i++) {
            if(hperm[i] != 1) {
		outdiv.innerHTML+=" not valid permute pattern, check for replicated destinations<br>";
		console.log(" died, not valid permute pattern");
		return 0;
	    }
	}
	invert_permute(tperm);
	if (DEBUG) {
	    outdiv.innerHTML+=" inverted perm<br>";
	    for(i=0; i < N; i++) { printint(tperm[i]); outdiv.innerHTML+=","; } outdiv.innerHTML+="<br>";
	}
	for(i=0; i < N; i++) {
	    if (REV_BUTTERFLY) {
		outperm[i] = tperm[i];
	    }
	    else { outperm[i] = brev(tperm[brev(i,logN)], logN); }
	}
	gen_switch_cntrl(toppath, bot, N, inperm, outperm, insb, outsb,0, 0);
	collect_switches(N, logN);
	convert_const_geo();
	if (DEBUG) {
	    print_geo_flow();
	}
	/*======================================================================*
	  BUTTERFLY
	  *======================================================================*/
	if (REV_BUTTERFLY) {
	    convert_rev_butterfly(); }
	else {
	    convert_butterfly();
	}
	if (DEBUG) {
	    outdiv.innerHTML+="<h3>butterfly flow = </h3>";
	    print_bfly_flow();
	}
	if (!REV_BUTTERFLY) {
	    for(i=0; i < N; i++) mux[i] = i;
	    for(j=0, stride=N/2; j < logN; j++, stride>>=1) {
		ki=0;
		for(k=0; k < N; k+= 2*stride) {
		    for(i=0; i < stride; i++)
		    {
			if(gsc[ki][j]) {
			    tmux[k+i]       = mux[k+i+stride];
			    tmux[k+i+stride]= mux[k+i];
			} else {
			    tmux[k+i]       = mux[k+i];
			    tmux[k+i+stride]= mux[k+i+stride];
			}
			ki++;
		    }
		}
		for(i=0; i < N; i++) {
		    mux[i] = tmux[i];
		    if(j==1) in128[i] = mux[i];
		}
	    }
	    for(j=logN, stride=2; j < 2*logN-1; j++, stride<<=1) {
		ki=0;
		for(k=0; k < N; k+= 2*stride) {
		    for(i=0; i < stride; i++)
		    {
			if(gsc[ki][j]) {
			    tmux[k+i]       = mux[k+i+stride];
			    tmux[k+i+stride]= mux[k+i];
			} else {
			    tmux[k+i]       = mux[k+i];
			    tmux[k+i+stride]= mux[k+i+stride];
			}
			ki++;
		    }
		}
		for(i=0; i < N; i++) {
		    mux[i] = tmux[i];
		    if(j==(2*logN-4)) out128[i] = mux[i];
		}
	    }
	    print_control_bits();
	    for(x=0;x < N; x++) inperm[x] = x;
	    for(x=0;x < N; x++) result[tperm[x]] = x;  //this is the pre modified for butterfly version
	    error = 0;
	    for(i=0; i < N; i++) {
		if (DEBUG) {
		    outdiv.innerHTML+="ac->"+mux[i]+" "+result[i]+"<ex<br>";
		}
		if(mux[i] != result[i]) { error = 1; }
	    }
	    if(error) { outdiv.innerHTML+="there were butterfly errors<br>"; }
	    else      { outdiv.innerHTML+="there were no butterfly errors<br>"; }
	    for(x=0;x < N; x+=32) { for(y=0;y < 32; y++) { outdiv.innerHTML+="vperm[x+y]"+", "; } outdiv.innerHTML+="<br>";}
	}
	else {
	    print_hvx_bits();
	    for(i=0; i < N; i++) mux[i] = i;
	    for(j=0, stride=1; j < logN; j++, stride<<=1) {
		ki=0;
		for(k=0; k < N; k+= 2*stride) {
		    for(i=0; i < stride; i++)
		    {
			if(gsc[ki][j]) {
			    tmux[k+i]       = mux[k+i+stride];
			    tmux[k+i+stride]= mux[k+i];
			} else {
			    tmux[k+i]       = mux[k+i];
			    tmux[k+i+stride]= mux[k+i+stride];
			}
			ki++;
		    }
		}
		for(i=0; i < N; i++) mux[i] = tmux[i];
	    }
	    for(j=logN, stride=Math.floor(N/4); j < 2*logN-1; j++, stride>>=1) {
		ki=0;
		for(k=0; k < N; k+= 2*stride) {
		    for(i=0; i < stride; i++)
		    {
			if(gsc[ki][j]) {
			    tmux[k+i]       = mux[k+i+stride];
			    tmux[k+i+stride]= mux[k+i];
			} else {
			    tmux[k+i]       = mux[k+i];
			    tmux[k+i+stride]= mux[k+i+stride];
			}
			ki++;
		    }
		}
		for(i=0; i < N; i++) mux[i] = tmux[i];
	    }
	    for(x=0;x < N; x++) inperm[x] = x;
	    for(x=0;x < N; x++) result[tperm[x]] = x;  //this is the pre modified for butterfly version
	    error = 0;
	    for(i=0; i < N; i++) {
		if (DEBUG) {
		    outdiv.innerHTML+="ac->"+mux[i]+" "+result[i]+"\<-ex <br>";
		}
		if(mux[i] != result[i]) { error = 1; }
	    }
	    if(error) {
		outdiv.innerHTML+="there were reverse butterfly errors<br>";
		console.log("there were reverse butterfly errors");}
	}
    }//end TEST
//    console.log("done");
    return false;
}

eval(runme)(arg);

}

/*======================================================================*/
/*                             end of file                              */
/*======================================================================*/
/*                       QUALCOMM                                       */
/*                                                                      */
/*                       Copyright (c) 2014                             */
/*                       All Rights Reserved                            */
/*                                                                      */
/*======================================================================*/
