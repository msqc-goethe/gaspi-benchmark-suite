/**
* @file stopwatch.h
* @brief Header fuer das Zeitmessungsmodul.
* @detail Mit diesem Modul kann die Ausfuehrungszeit
*         von Codeabschnitten gemessen werden. Hierzu stehen
*         zwei Messverfahren bereit, welche durch Definiton
*         von Makros bei der Compilation umgeschaltet werden
*         koennen. Hierzu werden die Makros
*         STOPWATCH_MODE_TOD und STOPWATCH_MODE_CLOCK
*         verwendet. Mit STOPWATCH_MODE_TOD wird die Fuktion
*         gettimeofday zur Zeitmessung verwendet; die
*         Messresultate sind in diesem Fall in Mikrosekunden.
*         Bei STOPWATCH_MODE_CLOCK werden ueber eine genauere Zeitmessung
*         Nanosekunden gezaehlt.
* @author Florian Beenen
* @version 2018-11-26
*/


#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__
#include <inttypes.h>
#include <stdio.h>

typedef uint64_t stopwatch_t;

#define STOPWATCH_TIME_ZERO 0


#define stopwatch_secsToUsecs(secs) ((secs) * (uint64_t)1000000)
#define stopwatch_secsToNsecs(secs) ((secs) * (uint64_t)1000000000)

/**
* Startet die Zeitmessung und liefert die aktuelle
* Messmetrik zurueck. Diese muss gespeichert werden
* und an 'stopwatch_stop' uebergeben werden, um die
* vergangene Zeit zu ermitteln.
*
* @return Aktuelle Messmetrik. Entweder in Mikrosekunden
*         oder in CPU-Zyklen.
*/
stopwatch_t stopwatch_start();

/**
* Stoppt die Zeitmessung und liefert die Differenz
* zum Startzeitpunkt in der jeweils ausgewaehlten
* Messeinheit zurueck.
*
* @param startTime Der Wert, der bei 'stopwatch_start'
*        erzeugt wurde.
* @return Differenz zum uebergebenen Wert in Mikrosekunden
*         oder CPU-Zyklen.
*/
stopwatch_t stopwatch_stop(stopwatch_t startTime);

/**
* Schreibt die mit 'stopwatch_stop' bezogene
* Zeitdifferenz mit der jeweiligen Einheit
* auf den uebergebenen Deskriptor.
*
* @param executionTime Ausfuehrungszeit von 'stopwatch_stop'.
* @param descriptor Dateideskriptor, auf den das Resultat geschrieben
*        werden soll.
*/
double stopwatch_getUsecs(stopwatch_t time);

#endif
