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

These pictograms can be switched on or off, or can change their appearance
as seen for volume settings.

LCDproc comes with icons that are integrated into the alphanumeric display
part, but is unaware of pictograms as described above.

One current workaround is to use the output() function to pass values to a
driver that uses these values to set pictograms. The downside is that any
application which wants to make use of these pictograms need to know the
exact type of display driver used and how that drivers handles values to the
output() function.

This document aims to specify client language and driver API extensions to
address pictograms in a manner not specific to certain drivers.

.. note::

   The common term for pictograms is indeed icons, but to distinguish the
   feature from LCDproc's already existing icon feature the term
   [Definition: pictogram] has been chosen.


Client language
===============

The LCDproc client language will be extended with two new functions that can
be called by a client.

pictogram_list
--------------

.. code::

   pictogram_list

If this function is called the server will respond with a list of supported
pictograms.

Clients are required to call this command first to retrieve the list of
supported pictograms. If the server does not support the pictogram feature,
it will respond with "huh? unknown command". Clients must not use
pictogram_set then.

If the server does support pictograms, it will respond with a list of
pictograms that are currently understood by at least one driver.

This means the list changes accordingly to the loaded drivers and may even
be empty if no loaded driver uses pictograms!

The response will look like:

.. code::

   pictograms [name:type[:group_id:group_type]]*

List entries are separated by a space character. Each list entry does consist
of 2 or 4 fields, separated by colon:

Name:
  The name by which the pictogram is known to the server. It is an US-ASCII
  string not containing any spaces.
Type:
  The type of the pictogram. This will be the string 'bool' for boolean-type
  or 'num' for numeric-type pictograms.
Group_id:
  This optional part contains a group identifier. The group_id is a number
  in the range 1-(2^32-1). It is used by the driver to inform the client
  that certain pictograms have a relation.
Group_type:
  The group_type must be present if a group_id is set. It describes how
  pictograms in the same group behave. It will be either the string 'alt'
  (alternate) for groups in which only one pictogram can be active at any
  time or 'mul' (multiple) for groups which can have several pictograms
  active at the same time.

Client shall process the strings case-insensitive.


pictogram_set
-------------

.. code::

   pictogram_set pictogram_name:value

By calling this command a single pictogram is changed. The pictogram is
referenced by its name and a value, according to the given type. The reserved
name 'all' can be used to turn off all pictograms at the same time.

.. note:: It has not yet been defined if 'all' can be used to turn on all
   pictograms at the same time.

For boolean-type pictograms the following values are recognized:

* String 'on' (case-insensitive) or value 1:

  This will turn the pictogram on.

* String 'off' (case-insensitive) or value 0:

  This will turn the pictogram off.

.. note:: Strings are to be used without the quotes.

For numeric-type pictograms value is a number between 0-1000 (inclusive).
The value 0 shall turn the pictogram off.


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

There will be no alternative implementation in LCDd core if a driver does
not understand a pictogram (like it is done for icons).

This function will never be called with multiple pictogram IDs combined into
one. It will be called for each pictogram to be set / cleared. If a driver
wishes to reduce communication with the hardware it may implement some kind
of cache and write the changes on a call to flush().

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


Internal implementation
=======================

This section should talk about processing of client messages, mapping of
pictogram names to pictogram IDs.
