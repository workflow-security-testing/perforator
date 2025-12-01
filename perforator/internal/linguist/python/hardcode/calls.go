package hardcode

var (
	// Core PyObject call functions: https://github.com/python/cpython/blob/3.12/Objects/call.c#L96

	// Usually these start with underscore
	CPythonInternalEvaluationFunctions = map[string]bool{
		"object_vacall":                 true,
		"callmethod":                    true,
		"_PyFunction_Vectorcall":        true,
		"_PyObject_Call":                true,
		"_PyObject_CallFunctionVa":      true,
		"_PyObject_CallFunction_SizeT":  true,
		"_PyObject_CallMethod":          true,
		"_PyObject_CallMethodId":        true,
		"_PyObject_CallMethodIdObjArgs": true,
		"_PyObject_CallMethodId_SizeT":  true,
		"_PyObject_CallMethodFormat":    true,
		"_PyObject_CallMethod_SizeT":    true,
		"_PyObject_Call_Prepend":        true,
		"_PyObject_FastCall":            true,
		"_PyObject_FastCallDictTstate":  true,
		"_PyObject_MakeTpCall":          true,
		"_PyVectorcall_Call":            true,
		"_PyVectorcall_NARGS":           true,
	}

	// These functions serve as an entrypoint to python evaluation loop
	CPythonEntryPointFunctions = map[string]bool{
		"_PyEval_EvalFrameDefault": true,
		"PyEval_EvalFrameEx":       true,
	}
)
