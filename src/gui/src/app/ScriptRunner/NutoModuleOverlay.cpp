/**\file
 * Overlay classes/methods from 'nuto' module
 */
/*
 * Written 2010 by Frank Richter <frank.richter@gmail.com>
 * For Bauhaus University Weimar, Institute of Structural Mechanics
 *
 */

#include <boost/python.hpp>
#include "NutoModuleOverlay.h"
#include "SwigConnect.h"

#include <wx/filename.h>
#include <wx/log.h>

#include <iostream>
#include <boost/python/raw_function.hpp>

#define ENABLE_VISUALIZE // @@@ Should get ENABLE_ flags from NUTO build
#include "nuto/visualize/VisualizeComponentBase.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"

using namespace boost::python;

NutoModuleOverlay& NutoModuleOverlay::GetOverlayInstance()
{
  // obtain NutoModuleOverlay instance
  object nuto_module = import ("nuto");
  object inst = nuto_module.attr("__nutoGuiOverlay__");
  
  return extract<NutoModuleOverlay&> (inst);
}

void NutoModuleOverlay::BindToPython ()
{
  class_<NutoModuleOverlay> ("_NutoModuleOverlay",
			     no_init)
    .def("ExportVtkDataFile", &NutoModuleOverlay::Overlay_ExportVtkDataFile)
    .staticmethod("ExportVtkDataFile")
  ;
}

void NutoModuleOverlay::Overlay_ExportVtkDataFile (const boost::python::object& self, const std::string& exportFile)
{
  {
    wxString loggedFN (exportFile);
    wxLogDebug (wxT ("intercepted ExportVtkDataFile() to %s"), loggedFN.c_str());
  }
  // 'self' contains nuto.Structure
  
  NuTo::Structure* nutoStruct;
  if (!SwigConnect::SwigExtract (nutoStruct, self.ptr(), "NuTo::Structure *"))
  {
    // @@@ Proper error feedback
    std::cerr << "couldn't extract NuTo::Structure" << std::endl;
    return;
  }
  
  /* Look for identifier of instance on which ExportVtkDataFile was called.
     Will be used to name result display */
  wxString resultsTitle;
  {
    dict locals (handle<> (borrowed (PyEval_GetLocals())) );
    list keys (locals.keys ());
    for (boost::python::ssize_t i = 0; i < len (keys); i++)
    {
      object key = keys[i];
      if (locals[key] == self)
      {
	std::string keyStr = extract<std::string> (key);
	resultsTitle = keyStr;
	break;
      }
    }
  }
  
  
  wxFileName newOutput;
  newOutput.AssignTempFileName (wxT ("nutogui"));
  if (!newOutput.IsOk())
  {
    // @@@ Proper error feedback
    std::cerr << "couldn't generate temp file name" << std::endl;
    return;
  }
  std::string outputPath (newOutput.GetFullPath().fn_str());
  nutoStruct->ExportVtkDataFile (outputPath);
  GetOverlayInstance().callback->ResultDataFile (newOutput.GetFullPath(), resultsTitle);
}
