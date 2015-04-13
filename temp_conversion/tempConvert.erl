%%**********************************************************************
%%* File:    tempConvert.erl
%%*
%%* Created: 1st March 2014
%%*
%%* Purpose: Implementation for Advanced Operating Systems Accessed
%%*          exercise 3. 
%%*
%%* Author:  Hector Grebbell
%%*
%%* Copyright: Copyright (c) Hector Grebbell 2014
%%*
%%* The contents of this file should not be copied or distributed
%%* because they are useless & I take no responsibility when they break.
%%*
%%* No rights reserved.
%%**********************************************************************

-module(tempConvert).
-compile(export_all).

% Starts All the Actors, becomes the display Actor
run() ->
	% Temperature Converter Actor
	TempC = spawn(tempConvert, tempConverter, []),
	% Sensor Actor Measuring in Celsius / Converts to Fahrenheit
	SenC = spawn(tempConvert, sensor, [self(), TempC, celsius]),
	% Sensor Actor Measuring in Fahrenheit / Converts to Celcius
	SenF = spawn(tempConvert, sensor, [self(), TempC, fahrenheit]),
	% Clock Actor - Signals SenC & SenF every second
	spawn(tempConvert, clock, [SenC, SenF]),
	% Display Actor - Receives and prints temperatures
	display(1).

% Receives messages from the sensors and prints out the temperature.
% Count exists as a sanity check
display(Count) ->
	receive
		{celsius, Val} ->
			io:format("~p. ~g~cC~n", [Count, Val, 176]),
			display(Count+1);
		{fahrenheit, Val} ->
			io:format("~p. ~g~cF~n", [Count, Val, 176]),
			display(Count+1)
	end.

% Takes a temperature and converts it
tempConverter() ->
	receive
		{From, convertToFahrenheit, Val} ->
			From ! {fahrenheit, ((Val * 1.8) + 32)},
			tempConverter();
		{From, convertToCelsius, Val} ->
			From ! {celsius, ((Val - 32) / 1.8)},
			tempConverter()
	end.

% Helper Functions since "convertToFahrenheit" etc doesn't make sense at
% a higher level but I wanted to be compliant with the handout
convertFrom(celsius) -> convertToFahrenheit;
convertFrom(fahrenheit) -> convertToCelsius.

% After a tick is received "Takes a reading" (uses the clock to generate
% fake data) & sends it to the temperature converter. Sends the response
% to the display.
sensor(Disp, TempConv, Type) ->
	receive
		{Type, _} ->
			io:format("what?~n"),
			exit("Converter returned same type");
		tick ->
			{_, Sec, _} = now(),	% TODO: CPU temp?
			TempConv ! {self(), convertFrom(Type), (Sec*997) rem 10000},
			sensor(Disp, TempConv, Type);
		{ConType, Val} ->
			Disp ! {ConType, Val},
			sensor(Disp, TempConv, Type)
	end.

% Every second messages the sensors
clock(FSen, CSen) ->
	timer:sleep(1000),
	FSen ! tick,
	CSen ! tick,
	clock(FSen, CSen).
