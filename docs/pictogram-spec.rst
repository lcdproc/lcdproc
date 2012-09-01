===================================
Pictogram Specification For LCDproc
===================================

:Author: Markus Dolze
:Contact: <bsdfan@nurfuerspam.de>

.. contents::


Introduction
============

In addition to alphanumeric characters represented by segments or pixels,
LCD/VFD displays are often capable to show additional pictograms, mostly
arranged around the alphanumeric display part.

These pictograms can be switched on or off, or by changing their appearance
as seen for volume settings.

LCDproc comes with icons that are integrated into the alphanumeric display
part, but is unaware of pictograms as described above.

One current workaround is to use the output() function to pass values to a
driver that uses these values to set pictograms. The downside is that any
application which wants to make use of these pictograms need to know the
exact type of display driver used and how that drivers handles values to the
output() function.

This document aims to specify client language and driver API extensions
address pictograms in a manner not specific to certain drivers.

.. note::

   The common term for pictograms is indeed icons, but to distinguish the
   feature from LCDproc's already existing icon feature the term
   [Definition: pictogram] has been chosen.


Client language
===============

This section should talk about the client language.


Driver API
==========

The driver API (struct lcd_logical_driver in lcd.h) will be extended with a
new function to process pictograms:

.. code:: c

   int (*set_pictogram)(unsigned int pictogramID, int value);


The pictogram function. The first parameter will be a pictogram number (ID)
and the second parameter will be its value. The function will return 0
(zero) if the pictogram is understood by the driver and -1 (minus one) if it
is not supported.

.. note::

   Their will be no alternative implementation in LCDd core if a driver does
   not understand a pictogram (like it is done for icons).

   This function will never be called with multiple pictogram IDs combined
   into one. It will be called for each pictogram to be set / cleared. If a
   driver wishes to reduce communication with the hardware it may implement
   some kind of cache and write the changes on a call to flush().

The pictogram ID is defined in the file called lcd_pictograms.h as an
enumerated type. It will have a symbolic name prefixed with [Definition:
PICT\_]. Drivers MUST NOT address pictograms by their ID number but only by
their symbolic name (e.g. within a switch statement) as the numbers are
subject to change.

There will be two types of pictograms:

Boolean type pictograms
  This type of pictograms can be turned on (visible) or off (not visible).
  Valid values are 0 (zero) which means off, or 1 (one) which means on.

Numeric type pictograms
  This type of pictograms will change its appearance according to the value
  assigned, e.g. a graph or WLAN strength indicator. Valid values are within
  the range 0-1000. A value of 0 (zero) turns the pictogram off.

There will be one reserved pictogram ID (symbolic name e.g. PICT_ALL) which
will be of boolean type and means that all pictograms the driver supports
shall be turned off or on.

.. note:: There are currently several options discussed on the mailing list,
   given here for reference:

   * There may be a third level for boolean type pictograms which does
     highlight it (e.g. make it more brighter or use a different color).

   * Values for numeric type may be negative. This allows pictograms to have
     an opposite direction, e.g. graphs growing from right-to-left instead of
     from left-to-right.

   * Drivers may define their local set of pictogram ID and assorted client
     language names.

   * Pictograms may be implemented as widgets. In this case LCDd core will
     keep track of its value.


Internal implementation
=======================

This section should talk about processing of client messages, mapping of
pictogram names to pictogram IDs.
