#version 430

#extension GL_ARB_compute_variable_group_size: enable

layout (local_size_variable) in;

// http://www.cvrl.org/ 390 - 749, 2° standard observer
const dvec3 lut[] = {
dvec3(3.769647E-03LF, 4.146161E-04LF, 1.847260E-02LF),
dvec3(4.532416E-03LF, 5.028333E-04LF, 2.221101E-02LF),
dvec3(5.446553E-03LF, 6.084991E-04LF, 2.669819E-02LF),
dvec3(6.538868E-03LF, 7.344436E-04LF, 3.206937E-02LF),
dvec3(7.839699E-03LF, 8.837389E-04LF, 3.847832E-02LF),
dvec3(9.382967E-03LF, 1.059646E-03LF, 4.609784E-02LF),
dvec3(1.120608E-02LF, 1.265532E-03LF, 5.511953E-02LF),
dvec3(1.334965E-02LF, 1.504753E-03LF, 6.575257E-02LF),
dvec3(1.585690E-02LF, 1.780493E-03LF, 7.822113E-02LF),
dvec3(1.877286E-02LF, 2.095572E-03LF, 9.276013E-02LF),
dvec3(2.214302E-02LF, 2.452194E-03LF, 1.096090E-01LF),
dvec3(2.601285E-02LF, 2.852216E-03LF, 1.290077E-01LF),
dvec3(3.043036E-02LF, 3.299115E-03LF, 1.512047E-01LF),
dvec3(3.544325E-02LF, 3.797466E-03LF, 1.764441E-01LF),
dvec3(4.109640E-02LF, 4.352768E-03LF, 2.049517E-01LF),
dvec3(4.742986E-02LF, 4.971717E-03LF, 2.369246E-01LF),
dvec3(5.447394E-02LF, 5.661014E-03LF, 2.725123E-01LF),
dvec3(6.223612E-02LF, 6.421615E-03LF, 3.117820E-01LF),
dvec3(7.070048E-02LF, 7.250312E-03LF, 3.547064E-01LF),
dvec3(7.982513E-02LF, 8.140173E-03LF, 4.011473E-01LF),
dvec3(8.953803E-02LF, 9.079860E-03LF, 4.508369E-01LF),
dvec3(9.974848E-02LF, 1.005608E-02LF, 5.034164E-01LF),
dvec3(1.104019E-01LF, 1.106456E-02LF, 5.586361E-01LF),
dvec3(1.214566E-01LF, 1.210522E-02LF, 6.162734E-01LF),
dvec3(1.328741E-01LF, 1.318014E-02LF, 6.760982E-01LF),
dvec3(1.446214E-01LF, 1.429377E-02LF, 7.378822E-01LF),
dvec3(1.566468E-01LF, 1.545004E-02LF, 8.013019E-01LF),
dvec3(1.687901E-01LF, 1.664093E-02LF, 8.655573E-01LF),
dvec3(1.808328E-01LF, 1.785302E-02LF, 9.295791E-01LF),
dvec3(1.925216E-01LF, 1.907018E-02LF, 9.921293E-01LF),
dvec3(2.035729E-01LF, 2.027369E-02LF, 1.051821),
dvec3(2.137531E-01LF, 2.144805E-02LF, 1.107509),
dvec3(2.231348E-01LF, 2.260041E-02LF, 1.159527),
dvec3(2.319245E-01LF, 2.374789E-02LF, 1.208869),
dvec3(2.403892E-01LF, 2.491247E-02LF, 1.256834),
dvec3(2.488523E-01LF, 2.612106E-02LF, 1.305008),
dvec3(2.575896E-01LF, 2.739923E-02LF, 1.354758),
dvec3(2.664991E-01LF, 2.874993E-02LF, 1.405594),
dvec3(2.753532E-01LF, 3.016909E-02LF, 1.456414),
dvec3(2.838921E-01LF, 3.165145E-02LF, 1.505960),
dvec3(2.918246E-01LF, 3.319038E-02LF, 1.552826),
dvec3(2.989200E-01LF, 3.477912E-02LF, 1.595902),
dvec3(3.052993E-01LF, 3.641495E-02LF, 1.635768),
dvec3(3.112031E-01LF, 3.809569E-02LF, 1.673573),
dvec3(3.169047E-01LF, 3.981843E-02LF, 1.710604),
dvec3(3.227087E-01LF, 4.157940E-02LF, 1.748280),
dvec3(3.288194E-01LF, 4.337098E-02LF, 1.787504),
dvec3(3.349242E-01LF, 4.517180E-02LF, 1.826609),
dvec3(3.405452E-01LF, 4.695420E-02LF, 1.863108),
dvec3(3.451688E-01LF, 4.868718E-02LF, 1.894332),
dvec3(3.482554E-01LF, 5.033657E-02LF, 1.917479),
dvec3(3.494153E-01LF, 5.187611E-02LF, 1.930529),
dvec3(3.489075E-01LF, 5.332218E-02LF, 1.934819),
dvec3(3.471746E-01LF, 5.470603E-02LF, 1.932650),
dvec3(3.446705E-01LF, 5.606335E-02LF, 1.926395),
dvec3(3.418483E-01LF, 5.743393E-02LF, 1.918437),
dvec3(3.390240E-01LF, 5.885107E-02LF, 1.910430),
dvec3(3.359926E-01LF, 6.030809E-02LF, 1.901224),
dvec3(3.324276E-01LF, 6.178644E-02LF, 1.889000),
dvec3(3.280157E-01LF, 6.326570E-02LF, 1.871996),
dvec3(3.224637E-01LF, 6.472352E-02LF, 1.848545),
dvec3(3.156225E-01LF, 6.614749E-02LF, 1.817792),
dvec3(3.078201E-01LF, 6.757256E-02LF, 1.781627),
dvec3(2.994771E-01LF, 6.904928E-02LF, 1.742514),
dvec3(2.909776E-01LF, 7.063280E-02LF, 1.702749),
dvec3(2.826646E-01LF, 7.238339E-02LF, 1.664439),
dvec3(2.747962E-01LF, 7.435960E-02LF, 1.629207),
dvec3(2.674312E-01LF, 7.659383E-02LF, 1.597360),
dvec3(2.605847E-01LF, 7.911436E-02LF, 1.568896),
dvec3(2.542749E-01LF, 8.195345E-02LF, 1.543823),
dvec3(2.485254E-01LF, 8.514816E-02LF, 1.522157),
dvec3(2.433039E-01LF, 8.872657E-02LF, 1.503611),
dvec3(2.383414E-01LF, 9.266008E-02LF, 1.486673),
dvec3(2.333253E-01LF, 9.689723E-02LF, 1.469595),
dvec3(2.279619E-01LF, 1.013746E-01LF, 1.450709),
dvec3(2.219781E-01LF, 1.060145E-01LF, 1.428440),
dvec3(2.151735E-01LF, 1.107377E-01LF, 1.401587),
dvec3(2.075619E-01LF, 1.155111E-01LF, 1.370094),
dvec3(1.992183E-01LF, 1.203122E-01LF, 1.334220),
dvec3(1.902290E-01LF, 1.251161E-01LF, 1.294275),
dvec3(1.806905E-01LF, 1.298957E-01LF, 1.250610),
dvec3(1.707154E-01LF, 1.346299E-01LF, 1.203696),
dvec3(1.604471E-01LF, 1.393309E-01LF, 1.154316),
dvec3(1.500244E-01LF, 1.440235E-01LF, 1.103284),
dvec3(1.395705E-01LF, 1.487372E-01LF, 1.051347),
dvec3(1.291920E-01LF, 1.535066E-01LF, 9.991789E-01LF),
dvec3(1.189859E-01LF, 1.583644E-01LF, 9.473958E-01LF),
dvec3(1.090615E-01LF, 1.633199E-01LF, 8.966222E-01LF),
dvec3(9.951424E-02LF, 1.683761E-01LF, 8.473981E-01LF),
dvec3(9.041850E-02LF, 1.735365E-01LF, 8.001576E-01LF),
dvec3(8.182895E-02LF, 1.788048E-01LF, 7.552379E-01LF),
dvec3(7.376817E-02LF, 1.841819E-01LF, 7.127879E-01LF),
dvec3(6.619477E-02LF, 1.896559E-01LF, 6.725198E-01LF),
dvec3(5.906380E-02LF, 1.952101E-01LF, 6.340976E-01LF),
dvec3(5.234242E-02LF, 2.008259E-01LF, 5.972433E-01LF),
dvec3(4.600865E-02LF, 2.064828E-01LF, 5.617313E-01LF),
dvec3(4.006154E-02LF, 2.121826E-01LF, 5.274921E-01LF),
dvec3(3.454373E-02LF, 2.180279E-01LF, 4.948809E-01LF),
dvec3(2.949091E-02LF, 2.241586E-01LF, 4.642586E-01LF),
dvec3(2.492140E-02LF, 2.307302E-01LF, 4.358841E-01LF),
dvec3(2.083981E-02LF, 2.379160E-01LF, 4.099313E-01LF),
dvec3(1.723591E-02LF, 2.458706E-01LF, 3.864261E-01LF),
dvec3(1.407924E-02LF, 2.546023E-01LF, 3.650566E-01LF),
dvec3(1.134516E-02LF, 2.640760E-01LF, 3.454812E-01LF),
dvec3(9.019658E-03LF, 2.742490E-01LF, 3.274095E-01LF),
dvec3(7.097731E-03LF, 2.850680E-01LF, 3.105939E-01LF),
dvec3(5.571145E-03LF, 2.964837E-01LF, 2.948102E-01LF),
dvec3(4.394566E-03LF, 3.085010E-01LF, 2.798194E-01LF),
dvec3(3.516303E-03LF, 3.211393E-01LF, 2.654100E-01LF),
dvec3(2.887638E-03LF, 3.344175E-01LF, 2.514084E-01LF),
dvec3(2.461588E-03LF, 3.483536E-01LF, 2.376753E-01LF),
dvec3(2.206348E-03LF, 3.629601E-01LF, 2.241211E-01LF),
dvec3(2.149559E-03LF, 3.782275E-01LF, 2.107484E-01LF),
dvec3(2.337091E-03LF, 3.941359E-01LF, 1.975839E-01LF),
dvec3(2.818931E-03LF, 4.106582E-01LF, 1.846574E-01LF),
dvec3(3.649178E-03LF, 4.277595E-01LF, 1.720018E-01LF),
dvec3(4.891359E-03LF, 4.453993E-01LF, 1.596918E-01LF),
dvec3(6.629364E-03LF, 4.635396E-01LF, 1.479415E-01LF),
dvec3(8.942902E-03LF, 4.821376E-01LF, 1.369428E-01LF),
dvec3(1.190224E-02LF, 5.011430E-01LF, 1.268279E-01LF),
dvec3(1.556989E-02LF, 5.204972E-01LF, 1.176796E-01LF),
dvec3(1.997668E-02LF, 5.401387E-01LF, 1.094970E-01LF),
dvec3(2.504698E-02LF, 5.600208E-01LF, 1.020943E-01LF),
dvec3(3.067530E-02LF, 5.800972E-01LF, 9.527993E-02LF),
dvec3(3.674999E-02LF, 6.003172E-01LF, 8.890075E-02LF),
dvec3(4.315171E-02LF, 6.206256E-01LF, 8.283548E-02LF),
dvec3(4.978584E-02LF, 6.409398E-01LF, 7.700982E-02LF),
dvec3(5.668554E-02LF, 6.610772E-01LF, 7.144001E-02LF),
dvec3(6.391651E-02LF, 6.808134E-01LF, 6.615436E-02LF),
dvec3(7.154352E-02LF, 6.999044E-01LF, 6.117199E-02LF),
dvec3(7.962917E-02LF, 7.180890E-01LF, 5.650407E-02LF),
dvec3(8.821473E-02LF, 7.351593E-01LF, 5.215121E-02LF),
dvec3(9.726978E-02LF, 7.511821E-01LF, 4.809566E-02LF),
dvec3(1.067504E-01LF, 7.663143E-01LF, 4.431720E-02LF),
dvec3(1.166192E-01LF, 7.807352E-01LF, 4.079734E-02LF),
dvec3(1.268468E-01LF, 7.946448E-01LF, 3.751912E-02LF),
dvec3(1.374060E-01LF, 8.082074E-01LF, 3.446846E-02LF),
dvec3(1.482471E-01LF, 8.213817E-01LF, 3.163764E-02LF),
dvec3(1.593076E-01LF, 8.340701E-01LF, 2.901901E-02LF),
dvec3(1.705181E-01LF, 8.461711E-01LF, 2.660364E-02LF),
dvec3(1.818026E-01LF, 8.575799E-01LF, 2.438164E-02LF),
dvec3(1.931090E-01LF, 8.682408E-01LF, 2.234097E-02LF),
dvec3(2.045085E-01LF, 8.783061E-01LF, 2.046415E-02LF),
dvec3(2.161166E-01LF, 8.879907E-01LF, 1.873456E-02LF),
dvec3(2.280650E-01LF, 8.975211E-01LF, 1.713788E-02LF),
dvec3(2.405015E-01LF, 9.071347E-01LF, 1.566174E-02LF),
dvec3(2.535441E-01LF, 9.169947E-01LF, 1.429644E-02LF),
dvec3(2.671300E-01LF, 9.269295E-01LF, 1.303702E-02LF),
dvec3(2.811351E-01LF, 9.366731E-01LF, 1.187897E-02LF),
dvec3(2.954164E-01LF, 9.459482E-01LF, 1.081725E-02LF),
dvec3(3.098117E-01LF, 9.544675E-01LF, 9.846470E-03LF),
dvec3(3.241678E-01LF, 9.619834E-01LF, 8.960687E-03LF),
dvec3(3.384319E-01LF, 9.684390E-01LF, 8.152811E-03LF),
dvec3(3.525786E-01LF, 9.738289E-01LF, 7.416025E-03LF),
dvec3(3.665839E-01LF, 9.781519E-01LF, 6.744115E-03LF),
dvec3(3.804244E-01LF, 9.814106E-01LF, 6.131421E-03LF),
dvec3(3.940988E-01LF, 9.836669E-01LF, 5.572778E-03LF),
dvec3(4.076972E-01LF, 9.852081E-01LF, 5.063463E-03LF),
dvec3(4.213484E-01LF, 9.863813E-01LF, 4.599169E-03LF),
dvec3(4.352003E-01LF, 9.875357E-01LF, 4.175971E-03LF),
dvec3(4.494206E-01LF, 9.890228E-01LF, 3.790291E-03LF),
dvec3(4.641616E-01LF, 9.910811E-01LF, 3.438952E-03LF),
dvec3(4.794395E-01LF, 9.934913E-01LF, 3.119341E-03LF),
dvec3(4.952180E-01LF, 9.959172E-01LF, 2.829038E-03LF),
dvec3(5.114395E-01LF, 9.980205E-01LF, 2.565722E-03LF),
dvec3(5.280233E-01LF, 9.994608E-01LF, 2.327186E-03LF),
dvec3(5.448696E-01LF, 9.999930E-01LF, 2.111280E-03LF),
dvec3(5.618898E-01LF, 9.997557E-01LF, 1.915766E-03LF),
dvec3(5.790137E-01LF, 9.989839E-01LF, 1.738589E-03LF),
dvec3(5.961882E-01LF, 9.979123E-01LF, 1.577920E-03LF),
dvec3(6.133784E-01LF, 9.967737E-01LF, 1.432128E-03LF),
dvec3(6.305897E-01LF, 9.957356E-01LF, 1.299781E-03LF),
dvec3(6.479223E-01LF, 9.947115E-01LF, 1.179667E-03LF),
dvec3(6.654866E-01LF, 9.935534E-01LF, 1.070694E-03LF),
dvec3(6.833782E-01LF, 9.921156E-01LF, 9.718623E-04LF),
dvec3(7.016774E-01LF, 9.902549E-01LF, 8.822531E-04LF),
dvec3(7.204110E-01LF, 9.878596E-01LF, 8.010231E-04LF),
dvec3(7.394495E-01LF, 9.849324E-01LF, 7.273884E-04LF),
dvec3(7.586285E-01LF, 9.815036E-01LF, 6.606347E-04LF),
dvec3(7.777885E-01LF, 9.776035E-01LF, 6.001146E-04LF),
dvec3(7.967750E-01LF, 9.732611E-01LF, 5.452416E-04LF),
dvec3(8.154530E-01LF, 9.684764E-01LF, 4.954847E-04LF),
dvec3(8.337389E-01LF, 9.631369E-01LF, 4.503642E-04LF),
dvec3(8.515493E-01LF, 9.571062E-01LF, 4.094455E-04LF),
dvec3(8.687862E-01LF, 9.502540E-01LF, 3.723345E-04LF),
dvec3(8.853376E-01LF, 9.424569E-01LF, 3.386739E-04LF),
dvec3(9.011588E-01LF, 9.336897E-01LF, 3.081396E-04LF),
dvec3(9.165278E-01LF, 9.242893E-01LF, 2.804370E-04LF),
dvec3(9.318245E-01LF, 9.146707E-01LF, 2.552996E-04LF),
dvec3(9.474524E-01LF, 9.052333E-01LF, 2.324859E-04LF),
dvec3(9.638388E-01LF, 8.963613E-01LF, 2.117772E-04LF),
dvec3(9.812596E-01LF, 8.883069E-01LF, 1.929758E-04LF),
dvec3(9.992953E-01LF, 8.808462E-01LF, 1.759024E-04LF),
dvec3(1.017343      , 8.736445E-01LF, 1.603947E-04LF),
dvec3(1.034790      , 8.663755E-01LF, 1.463059E-04LF),
dvec3(1.051011      , 8.587203E-01LF, 1.335031E-04LF),
dvec3(1.065522      , 8.504295E-01LF, 1.218660E-04LF),
dvec3(1.078421      , 8.415047E-01LF, 1.112857E-04LF),
dvec3(1.089944      , 8.320109E-01LF, 1.016634E-04LF),
dvec3(1.100320      , 8.220154E-01LF, 9.291003E-05LF),
dvec3(1.109767      , 8.115868E-01LF, 8.494468E-05LF),
dvec3(1.118438      , 8.007874E-01LF, 7.769425E-05LF),
dvec3(1.126266      , 7.896515E-01LF, 7.109247E-05LF),
dvec3(1.133138      , 7.782053E-01LF, 6.507936E-05LF),
dvec3(1.138952      , 7.664733E-01LF, 5.960061E-05LF),
dvec3(1.143620      , 7.544785E-01LF, 5.460706E-05LF),
dvec3(1.147095      , 7.422473E-01LF, 5.005417E-05LF),
dvec3(1.149464      , 7.298229E-01LF, 4.590157E-05LF),
dvec3(1.150838      , 7.172525E-01LF, 4.211268E-05LF),
dvec3(1.151326      , 7.045818E-01LF, 3.865437E-05LF),
dvec3(1.151033      , 6.918553E-01LF, 3.549661E-05LF),
dvec3(1.150002      , 6.791009E-01LF, 3.261220E-05LF),
dvec3(1.148061      , 6.662846E-01LF, 2.997643E-05LF),
dvec3(1.144998      , 6.533595E-01LF, 2.756693E-05LF),
dvec3(1.140622      , 6.402807E-01LF, 2.536339E-05LF),
dvec3(1.134757      , 6.270066E-01LF, 2.334738E-05LF),
dvec3(1.127298      , 6.135148E-01LF, 2.150221E-05LF),
dvec3(1.118342      , 5.998494E-01LF, 1.981268E-05LF),
dvec3(1.108033      , 5.860682E-01LF, 1.826500E-05LF),
dvec3(1.096515      , 5.722261E-01LF, 1.684667E-05LF),
dvec3(1.083928      , 5.583746E-01LF, 1.554631E-05LF),
dvec3(1.070387      , 5.445535E-01LF, 1.435360E-05LF),
dvec3(1.055934      , 5.307673E-01LF, 1.325915E-05LF),
dvec3(1.040592      , 5.170130E-01LF, 1.225443E-05LF),
dvec3(1.024385      , 5.032889E-01LF, 1.133169E-05LF),
dvec3(1.007344      , 4.895950E-01LF, 1.048387E-05LF),
dvec3(9.895268E-01LF, 4.759442E-01LF, 0.000000),
dvec3(9.711213E-01LF, 4.623958E-01LF, 0.000000),
dvec3(9.523257E-01LF, 4.490154E-01LF, 0.000000),
dvec3(9.333248E-01LF, 4.358622E-01LF, 0.000000),
dvec3(9.142877E-01LF, 4.229897E-01LF, 0.000000),
dvec3(8.952798E-01LF, 4.104152E-01LF, 0.000000),
dvec3(8.760157E-01LF, 3.980356E-01LF, 0.000000),
dvec3(8.561607E-01LF, 3.857300E-01LF, 0.000000),
dvec3(8.354235E-01LF, 3.733907E-01LF, 0.000000),
dvec3(8.135565E-01LF, 3.609245E-01LF, 0.000000),
dvec3(7.904565E-01LF, 3.482860E-01LF, 0.000000),
dvec3(7.664364E-01LF, 3.355702E-01LF, 0.000000),
dvec3(7.418777E-01LF, 3.228963E-01LF, 0.000000),
dvec3(7.171219E-01LF, 3.103704E-01LF, 0.000000),
dvec3(6.924717E-01LF, 2.980865E-01LF, 0.000000),
dvec3(6.681600E-01LF, 2.861160E-01LF, 0.000000),
dvec3(6.442697E-01LF, 2.744822E-01LF, 0.000000),
dvec3(6.208450E-01LF, 2.631953E-01LF, 0.000000),
dvec3(5.979243E-01LF, 2.522628E-01LF, 0.000000),
dvec3(5.755410E-01LF, 2.416902E-01LF, 0.000000),
dvec3(5.537296E-01LF, 2.314809E-01LF, 0.000000),
dvec3(5.325412E-01LF, 2.216378E-01LF, 0.000000),
dvec3(5.120218E-01LF, 2.121622E-01LF, 0.000000),
dvec3(4.922070E-01LF, 2.030542E-01LF, 0.000000),
dvec3(4.731224E-01LF, 1.943124E-01LF, 0.000000),
dvec3(4.547417E-01LF, 1.859227E-01LF, 0.000000),
dvec3(4.368719E-01LF, 1.778274E-01LF, 0.000000),
dvec3(4.193121E-01LF, 1.699654E-01LF, 0.000000),
dvec3(4.018980E-01LF, 1.622841E-01LF, 0.000000),
dvec3(3.844986E-01LF, 1.547397E-01LF, 0.000000),
dvec3(3.670592E-01LF, 1.473081E-01LF, 0.000000),
dvec3(3.497167E-01LF, 1.400169E-01LF, 0.000000),
dvec3(3.326305E-01LF, 1.329013E-01LF, 0.000000),
dvec3(3.159341E-01LF, 1.259913E-01LF, 0.000000),
dvec3(2.997374E-01LF, 1.193120E-01LF, 0.000000),
dvec3(2.841189E-01LF, 1.128820E-01LF, 0.000000),
dvec3(2.691053E-01LF, 1.067113E-01LF, 0.000000),
dvec3(2.547077E-01LF, 1.008052E-01LF, 0.000000),
dvec3(2.409319E-01LF, 9.516653E-02LF, 0.000000),
dvec3(2.277792E-01LF, 8.979594E-02LF, 0.000000),
dvec3(2.152431E-01LF, 8.469044E-02LF, 0.000000),
dvec3(2.033010E-01LF, 7.984009E-02LF, 0.000000),
dvec3(1.919276E-01LF, 7.523372E-02LF, 0.000000),
dvec3(1.810987E-01LF, 7.086061E-02LF, 0.000000),
dvec3(1.707914E-01LF, 6.671045E-02LF, 0.000000),
dvec3(1.609842E-01LF, 6.277360E-02LF, 0.000000),
dvec3(1.516577E-01LF, 5.904179E-02LF, 0.000000),
dvec3(1.427936E-01LF, 5.550703E-02LF, 0.000000),
dvec3(1.343737E-01LF, 5.216139E-02LF, 0.000000),
dvec3(1.263808E-01LF, 4.899699E-02LF, 0.000000),
dvec3(1.187979E-01LF, 4.600578E-02LF, 0.000000),
dvec3(1.116088E-01LF, 4.317885E-02LF, 0.000000),
dvec3(1.047975E-01LF, 4.050755E-02LF, 0.000000),
dvec3(9.834835E-02LF, 3.798376E-02LF, 0.000000),
dvec3(9.224597E-02LF, 3.559982E-02LF, 0.000000),
dvec3(8.647506E-02LF, 3.334856E-02LF, 0.000000),
dvec3(8.101986E-02LF, 3.122332E-02LF, 0.000000),
dvec3(7.586514E-02LF, 2.921780E-02LF, 0.000000),
dvec3(7.099633E-02LF, 2.732601E-02LF, 0.000000),
dvec3(6.639960E-02LF, 2.554223E-02LF, 0.000000),
dvec3(6.206225E-02LF, 2.386121E-02LF, 0.000000),
dvec3(5.797409E-02LF, 2.227859E-02LF, 0.000000),
dvec3(5.412533E-02LF, 2.079020E-02LF, 0.000000),
dvec3(5.050600E-02LF, 1.939185E-02LF, 0.000000),
dvec3(4.710606E-02LF, 1.807939E-02LF, 0.000000),
dvec3(4.391411E-02LF, 1.684817E-02LF, 0.000000),
dvec3(4.091411E-02LF, 1.569188E-02LF, 0.000000),
dvec3(3.809067E-02LF, 1.460446E-02LF, 0.000000),
dvec3(3.543034E-02LF, 1.358062E-02LF, 0.000000),
dvec3(3.292138E-02LF, 1.261573E-02LF, 0.000000),
dvec3(3.055672E-02LF, 1.170696E-02LF, 0.000000),
dvec3(2.834146E-02LF, 1.085608E-02LF, 0.000000),
dvec3(2.628033E-02LF, 1.006476E-02LF, 0.000000),
dvec3(2.437465E-02LF, 9.333376E-03LF, 0.000000),
dvec3(2.262306E-02LF, 8.661284E-03LF, 0.000000),
dvec3(2.101935E-02LF, 8.046048E-03LF, 0.000000),
dvec3(1.954647E-02LF, 7.481130E-03LF, 0.000000),
dvec3(1.818727E-02LF, 6.959987E-03LF, 0.000000),
dvec3(1.692727E-02LF, 6.477070E-03LF, 0.000000),
dvec3(1.575417E-02LF, 6.027677E-03LF, 0.000000),
dvec3(1.465854E-02LF, 5.608169E-03LF, 0.000000),
dvec3(1.363571E-02LF, 5.216691E-03LF, 0.000000),
dvec3(1.268205E-02LF, 4.851785E-03LF, 0.000000),
dvec3(1.179394E-02LF, 4.512008E-03LF, 0.000000),
dvec3(1.096778E-02LF, 4.195941E-03LF, 0.000000),
dvec3(1.019964E-02LF, 3.902057E-03LF, 0.000000),
dvec3(9.484317E-03LF, 3.628371E-03LF, 0.000000),
dvec3(8.816851E-03LF, 3.373005E-03LF, 0.000000),
dvec3(8.192921E-03LF, 3.134315E-03LF, 0.000000),
dvec3(7.608750E-03LF, 2.910864E-03LF, 0.000000),
dvec3(7.061391E-03LF, 2.701528E-03LF, 0.000000),
dvec3(6.549509E-03LF, 2.505796E-03LF, 0.000000),
dvec3(6.071970E-03LF, 2.323231E-03LF, 0.000000),
dvec3(5.627476E-03LF, 2.153333E-03LF, 0.000000),
dvec3(5.214608E-03LF, 1.995557E-03LF, 0.000000),
dvec3(4.831848E-03LF, 1.849316E-03LF, 0.000000),
dvec3(4.477579E-03LF, 1.713976E-03LF, 0.000000),
dvec3(4.150166E-03LF, 1.588899E-03LF, 0.000000),
dvec3(3.847988E-03LF, 1.473453E-03LF, 0.000000),
dvec3(3.569452E-03LF, 1.367022E-03LF, 0.000000),
dvec3(3.312857E-03LF, 1.268954E-03LF, 0.000000),
dvec3(3.076022E-03LF, 1.178421E-03LF, 0.000000),
dvec3(2.856894E-03LF, 1.094644E-03LF, 0.000000),
dvec3(2.653681E-03LF, 1.016943E-03LF, 0.000000),
dvec3(2.464821E-03LF, 9.447269E-04LF, 0.000000),
dvec3(2.289060E-03LF, 8.775171E-04LF, 0.000000),
dvec3(2.125694E-03LF, 8.150438E-04LF, 0.000000),
dvec3(1.974121E-03LF, 7.570755E-04LF, 0.000000),
dvec3(1.833723E-03LF, 7.033755E-04LF, 0.000000),
dvec3(1.703876E-03LF, 6.537050E-04LF, 0.000000),
dvec3(1.583904E-03LF, 6.078048E-04LF, 0.000000),
dvec3(1.472939E-03LF, 5.653435E-04LF, 0.000000),
dvec3(1.370151E-03LF, 5.260046E-04LF, 0.000000),
dvec3(1.274803E-03LF, 4.895061E-04LF, 0.000000),
dvec3(1.186238E-03LF, 4.555970E-04LF, 0.000000),
dvec3(1.103871E-03LF, 4.240548E-04LF, 0.000000),
dvec3(1.027194E-03LF, 3.946860E-04LF, 0.000000),
dvec3(9.557493E-04LF, 3.673178E-04LF, 0.000000),
dvec3(8.891262E-04LF, 3.417941E-04LF, 0.000000),
dvec3(8.269535E-04LF, 3.179738E-04LF, 0.000000),
dvec3(7.689351E-04LF, 2.957441E-04LF, 0.000000),
dvec3(7.149425E-04LF, 2.750558E-04LF, 0.000000),
dvec3(6.648590E-04LF, 2.558640E-04LF, 0.000000),
dvec3(6.185421E-04LF, 2.381142E-04LF, 0.000000),
dvec3(5.758303E-04LF, 2.217445E-04LF, 0.000000),
dvec3(5.365046E-04LF, 2.066711E-04LF, 0.000000),
dvec3(5.001842E-04LF, 1.927474E-04LF, 0.000000),
dvec3(4.665005E-04LF, 1.798315E-04LF, 0.000000),
dvec3(4.351386E-04LF, 1.678023E-04LF, 0.000000),
dvec3(4.058303E-04LF, 1.565566E-04LF, 0.000000),
dvec3(3.783733E-04LF, 1.460168E-04LF, 0.000000),
dvec3(3.526892E-04LF, 1.361535E-04LF, 0.000000),
dvec3(3.287199E-04LF, 1.269451E-04LF, 0.000000),
dvec3(3.063998E-04LF, 1.183671E-04LF, 0.000000)
};

// TODO make configurable
const uint TEX_WIDTH = 256u;
const uint TEX_HEIGHT = TEX_WIDTH;

// TODO make configurable
const uint MIN_WAVELENGTH = 390u;
const uint MAX_WAVELENGTH = 749u;
const uint NUM_WAVELENGTHS = MAX_WAVELENGTH - MIN_WAVELENGTH + 1;

// Size: 2880 bytes -> ~350,000 pixels per available gigabyte of ram
struct Pixel {
    double intensityAtWavelengths[NUM_WAVELENGTHS];
};

layout(std430, binding = 0) buffer Pixels {
    // Pixel pixels[TEX_WIDTH][TEX_HEIGHT];
    Pixel[] pixels;
};

layout(std430, binding = 1) buffer PixelOutput {
    dvec4[] pixelOutput;
};

Pixel getPixel(uvec2 position) {
    uint idx = position.y * TEX_WIDTH + position.x;
    return pixels[idx];
}

void setOutputPixel(uvec2 position, dvec4 color) {
    pixelOutput[position.y * TEX_WIDTH + position.x] = color;
}

dvec3 spectralDistributionToXYZ(double spectrum[NUM_WAVELENGTHS]) {
    dvec3 xyz = dvec3(0.0LF);
    /*for (uint i = 0u; i < NUM_WAVELENGTHS; ++i) {
        xyz += spectrum[i] * lut[i];
    }*/

    // For some mysterious reason the loop doesn't work with double precision.
    // So it has to be unrolled manually...
    {
        uint i = 0u;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
        xyz += spectrum[i] * lut[i]; i++;
    }

    return xyz;
}

const dmat3 CIE_RGB_E = dmat3(
     2.3706743LF, -0.9000405LF, -0.4706338LF,
    -0.5138850LF,  1.4253036LF,  0.0885814LF,
     0.0052982LF, -0.0146949LF,  1.0093968LF
);

const dmat3 CIE_RGB_D50 = dmat3(
     2.3638081LF, -0.8676030LF, -0.4988161LF,
    -0.5005940LF,  1.3962369LF,  0.1047562LF,
     0.0141712LF, -0.0306400LF,  1.2323842LF
);

const dmat3 SRGB_D65 = dmat3(
     3.2404542LF, -1.5371385LF, -0.4985314LF,
    -0.9692660LF,  1.8760108LF,  0.0415560LF,
     0.0556434LF, -0.2040259LF,  1.0572252LF
);

const dmat3 SRGB_D50 = dmat3(
     3.1338561LF, -1.6168667LF, -0.4906146LF,
    -0.9787684LF,  1.9161415LF,  0.0334540LF,
     0.0719453LF, -0.2289914LF,  1.4052427LF
);

const dmat3 ADOBE_D65 = dmat3(
     2.0413690LF, -0.5649464LF, -0.3446944LF,
    -0.9692660LF,  1.8760108LF,  0.0415560LF,
     0.0134474LF, -0.1183897LF,  1.0154096LF
);

const dmat3 CM_D50 = dmat3(
     2.6422874LF, -1.2234270LF, -0.3930143LF,
    -1.1119763LF,  2.0590183LF,  0.0159614LF,
     0.0821699LF, -0.2807254LF,  1.4559877LF
);

dvec3 xyzToRGB(dvec3 xyz) {
    return xyz * SRGB_D65;
}



dvec4 calcColor(uvec2 position) {
    Pixel p = getPixel(position);

    dvec3 xyz = spectralDistributionToXYZ(p.intensityAtWavelengths);
    dvec3 rgb = xyzToRGB(xyz);

    return dvec4(max(rgb.x, 0.0LF), max(rgb.y, 0.0LF), max(rgb.z, 0.0LF), 1.0LF);
}

void main() {
    uvec2 gid = gl_GlobalInvocationID.xy;
    dvec4 rgb = calcColor(gid);

    setOutputPixel(gid, dvec4(rgb.r, rgb.g, rgb.b, 1.0LF));
}