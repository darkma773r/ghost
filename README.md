Ghost
=====

Ghost is a simple program for adding opacity hints to 
X11 windows based on configurable rules. I
wrote this program after trying out the [TWM](http://xwinman.org/vtwm.php) window manager
with xcompmgr and failing to find a simple way to add
opacity to xterm windows. [transset-df](http://forchheimer.se/transset-df/)
worked but did not give me the customization options that I
wanted. Ghost allows a user to specify opacity settings for
any X window. It can be run in single-pass mode where settings are applied
once to existing windows and in monitor mode where the program
continuously monitors X events and applies opacity settings
as needed. Different settings can be applied for focused
and unfocused states in monitor mode.

Ghost uses the [XCB library](http://xcb.freedesktop.org/). 

Usage
-----

*ghost [OPTIONS] [rule string]*

### Options
**-m, --monitor**	
Enters monitoring mode, where X events are tracked and opacity settings
applied as needed. Different settings can be applied for focused and unfocused windows in this mode.
Without this switch, only "normal" opacity settings are used.

**-f, --file**		
If given, the next argument is interpreted as the name of a file
containing the opacity rules for Ghost. If not given, opacity rules must be given directly as a
string after all other arguments.	

### Examples
**ghost -f rules.txt**

Applies "normal" opacity settings given in the file rules.txt
to existing X windows.
	
**ghost -m "WM_CLASS(xterm){f:0.8;n:0.6;}"** 		

Monitors X events and applies "focus" and "normal" settings to windows
with a class of "xterm" until the program is killed.

**ghost -m -f rules.txt**

Monitors X events and applies the settings given in the file
rules.txt to matching windows until the program is killed.

Rules
-----

Rules are written for Ghost in a format similar to CSS:
a series of matchers specifying
X string properties and corresponding values (not case-sensitive)
are given before a set of focus and normal opacities in
braces. Properties and values are specified with the name of
the X property followed by the desired value in parantheses.
Whitespace between two matchers represents a logical AND 
(higher precedence) while a comma represents a logical OR
(lower precendence). 
Example: The following rule
applies an opacity of 0.8 to focused windows and 0.6 to unfocused
windows that have either a class of "xterm" and name of "home"
or a class of "thunar":

WM_CLASS( xterm ) WM_NAME( home ),
WM_CLASS( thunar ) {
	focus: 0.8;
	normal: 0.6;
} 

Lines starting with '#' are considered comments. String tokens
can be surrounded with single or double quotes to 
allow strings with whitespace or non-alphanumeric
characters. Ex: 'WM_CLASS'( "some class" ) 

The opacity settings "focus" and "normal" can be abbreviated
with "f" and "n".

Possible X property names and values can be found using the
[xprop](http://linux.die.net/man/1/xprop) utility.
