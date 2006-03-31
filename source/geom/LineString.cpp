/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/util/IllegalArgumentException.h> 
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/operation/IsSimpleOp.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h> // for getBoundary
#include <geos/geom/Envelope.h>

#include <algorithm>
#include <typeinfo>
#include <cassert>

using namespace std;
using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

LineString::LineString(const LineString &ls)
	:
	Geometry(ls.getFactory())
{
	points=ls.points->clone();
}

LineString*
LineString::reverse() const
{
	CoordinateSequence* seq = points->clone();
	CoordinateSequence::reverse(seq);
	return getFactory()->createLineString(seq);
}


/**
 * Constructs a <code>LineString</code> taking ownership of the
 * given CoordinateSequence.
 *
 * @param newCoords the list of coordinates making up the linestring,
 *	or <code>null</code> to create the empty geometry.
 *	Consecutive points may not be equal.
 *
 * @param factory the GeometryFactory used to create this Geometry.
 *
 */  
LineString::LineString(CoordinateSequence *newCoords,
		const GeometryFactory *factory)
	:
	Geometry(factory)
{
	if (newCoords==NULL) {
		points=factory->getCoordinateSequenceFactory()->create(NULL);
		return;
	}
	if (newCoords->getSize()==1) {
		throw util::IllegalArgumentException("point array must contain 0 or >1 elements\n");
	}
	points=newCoords;
}


LineString::~LineString()
{
	delete points;
}

CoordinateSequence*
LineString::getCoordinates() const
{
	assert(points);
	return points->clone();
	//return points;
}

const CoordinateSequence*
LineString::getCoordinatesRO() const
{
	return points;
}

const Coordinate&
LineString::getCoordinateN(int n) const
{
	assert(points);
	return points->getAt(n);
}

int
LineString::getDimension() const
{
	return 1;
}

int
LineString::getBoundaryDimension() const
{
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

bool
LineString::isEmpty() const
{
	assert(points);
	return points->getSize()==0;
}

int
LineString::getNumPoints() const
{
	assert(points);
	return points->getSize();
}

Point*
LineString::getPointN(int n) const
{
	return getFactory()->createPoint(points->getAt(n));
}

Point*
LineString::getStartPoint() const
{
	if (isEmpty()) {
		return NULL;
		//return new Point(NULL,NULL);
	}
	return getPointN(0);
}

Point*
LineString::getEndPoint() const
{
	if (isEmpty()) {
		return NULL;
		//return new Point(NULL,NULL);
	}
	return getPointN(getNumPoints() - 1);
}

bool
LineString::isClosed() const
{
	if (isEmpty()) {
		return false;
	}
	return getCoordinateN(0).equals2D(getCoordinateN(getNumPoints()-1));
}

bool
LineString::isRing() const
{
	return isClosed() && isSimple();
}

string
LineString::getGeometryType() const
{
	return "LineString";
}

bool
LineString::isSimple() const
{
	operation::IsSimpleOp iso;
	return iso.isSimple(this); 
}

Geometry*
LineString::getBoundary() const
{
	if (isEmpty()) {
		return getFactory()->createEmptyGeometry();
	}
	if (isClosed()) {
		return getFactory()->createMultiPoint();
	}
	vector<Geometry*> *pts=new vector<Geometry*>();
	pts->push_back(getStartPoint());
	pts->push_back(getEndPoint());
	MultiPoint *mp = getFactory()->createMultiPoint(pts);
	return mp;
}

bool
LineString::isCoordinate(Coordinate& pt) const
{
	assert(points);
	int npts=points->getSize();
	for (int i = 0; i<npts; i++) {
		if (points->getAt(i)==pt) {
			return true;
		}
	}
	return false;
}

Envelope*
LineString::computeEnvelopeInternal() const
{
	if (isEmpty()) {
		// Should return NULL instead ?
		return new Envelope();
	}

	assert(points);
	const Coordinate& c=points->getAt(0);
	double minx = c.x;
	double miny = c.y;
	double maxx = c.x;
	double maxy = c.y;
	int npts=points->getSize();
	for (int i=1; i<npts; i++) {
		const Coordinate &c=points->getAt(i);
		minx = minx < c.x ? minx : c.x;
		maxx = maxx > c.x ? maxx : c.x;
		miny = miny < c.y ? miny : c.y;
		maxy = maxy > c.y ? maxy : c.y;
	}

	// Shouldn't we be caching Envelopes ?? --strk;
	return new Envelope(minx, maxx, miny, maxy);
}

bool
LineString::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}

	const LineString *otherLineString=dynamic_cast<const LineString*>(other);
	assert(otherLineString);
	unsigned int npts=points->getSize();
	if (npts!=otherLineString->points->getSize()) {
		return false;
	}
	for (unsigned int i=0; i<npts; ++i) {
		if (!equal(points->getAt(i),otherLineString->points->getAt(i),tolerance)) {
			return false;
		}
	}
	return true;
}

void
LineString::apply_rw(const CoordinateFilter *filter)
{
	assert(points);
	points->apply_rw(filter);
}

void
LineString::apply_ro(CoordinateFilter *filter) const
{
	assert(points);
	points->apply_ro(filter);
}

void LineString::apply_rw(GeometryFilter *filter)
{
	assert(filter);
	filter->filter_rw(this);
}

void LineString::apply_ro(GeometryFilter *filter) const
{
	assert(filter);
	filter->filter_ro(this);
}

/*public*/
void
LineString::normalize()
{
	assert(points);
	int npts=points->getSize();
	int n=npts/2;
	for (int i=0; i<n; i++) {
		int j = npts - 1 - i;
		if (!(points->getAt(i)==points->getAt(j))) {
			if (points->getAt(i).compareTo(points->getAt(j)) > 0) {
				CoordinateSequence::reverse(points);
			}
			return;
		}
	}
}

int
LineString::compareToSameClass(const Geometry *ls) const
{
	assert(dynamic_cast<const LineString*>(ls));
	const LineString *line=static_cast<const LineString*>(ls);
	// MD - optimized implementation
	int mynpts=points->getSize();
	int othnpts=line->points->getSize();
	if ( mynpts > othnpts ) return 1;
	if ( mynpts < othnpts ) return -1;
	for (int i=0; i<mynpts; i++)
	{
		int cmp=points->getAt(i).compareTo(line->points->getAt(i));
		if (cmp) return cmp;
	}
	return 0;
}

const Coordinate*
LineString::getCoordinate() const
{
	if (isEmpty()) return NULL; 
	return &(points->getAt(0));
}

double
LineString::getLength() const
{
	return CGAlgorithms::length(points);
}

void
LineString::apply_rw(GeometryComponentFilter *filter)
{
	assert(filter);
	filter->filter_rw(this);
}

void
LineString::apply_ro(GeometryComponentFilter *filter) const
{
	assert(filter);
	filter->filter_ro(this);
}

GeometryTypeId
LineString::getGeometryTypeId() const
{
	return GEOS_LINESTRING;
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.63  2006/03/31 16:55:17  strk
 * Added many assertions checking in LineString implementation.
 * Changed ::getCoordinate() to return NULL on empty geom.
 * Changed ::get{Start,End}Point() to return NULL on empty geom.
 *
 * Revision 1.62  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.61  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.60  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.59  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.58  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.57  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.56  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
 * Revision 1.55  2006/01/31 19:07:33  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.54  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.53  2005/12/07 22:52:03  strk
 * Added CoordinateSequence::apply_rw(CoordinateFilter *) and
 * CoordinateSequence::apply_ro(CoordinateFilter *) const
 * to reduce coordinate copies on read-write CoordinateFilter
 * applications (previously required getAt()/setAt() calls).
 * Undefined PROFILE_COORDINATE_COPIES (erroneously left defined by previous commit)
 *
 * Revision 1.52  2005/11/28 18:37:32  strk
 * Minor warning removal
 *
 * Revision 1.51  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.50  2005/11/15 10:02:27  strk
 * optimized envelope computation reducing virtual calls
 *
 * Revision 1.49  2005/11/10 09:33:17  strk
 * Removed virtual overloading LineString::compareTo(LineString *)
 *
 * Revision 1.48  2005/06/23 14:22:33  strk
 * Inlined and added missing ::clone() for Geometry subclasses
 *
 * Revision 1.47  2005/02/22 17:10:47  strk
 * Reduced CoordinateSequence::getSize() calls.
 *
 * Revision 1.46  2004/12/03 22:52:56  strk
 * enforced const return of CoordinateSequence::toVector() method to derivate classes.
 *
 * Revision 1.45  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.44  2004/09/13 09:07:28  strk
 * Ported fix in LineString::isCoordinate
 *
 * Revision 1.43  2004/09/12 03:51:27  pramsey
 * Casting changes to allow OS/X compilation.
 *
 * Revision 1.42  2004/07/22 08:45:50  strk
 * Documentation updates, memory leaks fixed.
 *
 * Revision 1.41  2004/07/19 13:19:30  strk
 * Documentation fixes
 *
 * Revision 1.40  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.39  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.38  2004/07/05 14:23:03  strk
 * More documentation cleanups.
 *
 * Revision 1.37  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.36  2004/07/03 12:51:37  strk
 * Documentation cleanups for DoxyGen.
 *
 * Revision 1.35  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.34  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.33  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.32  2004/06/15 20:38:44  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.31  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateSequence.
 *
 * Revision 1.30  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.29  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.28  2004/04/10 22:41:24  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.27  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.26  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.25  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.24  2003/10/31 16:36:04  strk
 * Re-introduced clone() method. Copy constructor could not really replace it.
 *
 * Revision 1.23  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 * Revision 1.22  2003/10/15 09:54:29  strk
 * Added getCoordinatesRO() public method.
 *
 **********************************************************************/

