f = 0.75e6;
t_low = 0.2e-6;
Vin = 4.7;      % Input voltage in volts
Vout = 3.0;       % Output voltage in volts
Iout = 1;       % Output current in amps
R = Vout/Iout;  % Load resistance in ohms
Cout = 200e-6;   % Output capacitance in farads
R_c = 1e-3;     % Output capacitor ESR in ohms
L = 10e-6;     % Inductor value in henries
R_l = 17.2e-3;    % Inductor ESR in ohms
R_MOSFET = 50e-3;   % Average MOSFET switching resistance.  Taken from graph in datasheet
R_s = 2*R_MOSFET + R_l; % Average series resistance of power stage

% Left-half plane zero frequency
f_z = 1/(2*pi*R_c*Cout);

% Right-half plane zero (only applies to boost mode)
f_RHPZ = R*(1-t_low*f)^2*Vin^2/(2*pi*L*Vout^2);

% Boost mode Q-factor
Q_boost = sqrt(L*Cout*R*(R_s + R*Vin^2/Vout^2))/(L+Cout*R_s*R);

% Boost mode gain
G_boost = 2*Vout^2/Vin;

% Boost mode resonant frequency
f0_boost = 1/(2*pi)*sqrt((R_s + R*Vin^2/Vout^2)/(L*Cout*(R+R_c)) );
% f0_boost = 1/(2*pi)*Vin/Vout*sqrt(1/(L*Cout));

% BUCK MODE
f0_buck = 1/(2*pi)*sqrt((R+R_s)/(L*Cout*(R+R_c)));

Q_buck = sqrt(L*Cout*(R+R_c)*(R+R_s))/(R*R_c*Cout+L+Cout*R_s*(R+R_c));

G_buck = 2*Vin*R/(R+R_s);

fprintf('\nCOMPUTED PARAMETERS FOR BUCK-BOOST DESIGN\n\n');
fprintf('f_z = %3.6f\n',f_z);
fprintf('BOOST f_RHPZ = %3.6f\n',f_RHPZ);
fprintf('BOOST G = %3.6f\n',G_boost);
fprintf('BOOST f0 = %3.6f\n',f0_boost);
fprintf('BOOST Q = %3.6f\n',Q_boost);
fprintf('BUCK f0 = %3.6f\n',f0_buck);
fprintf('BUCK Q = %3.6f\n',Q_buck);
fprintf('BUCK G = %3.6f\n',G_buck);

