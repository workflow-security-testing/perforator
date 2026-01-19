package polyheapprof

import "net/http"

func ServeCurrentHeapProfile(w http.ResponseWriter, r *http.Request) {
	p, err := ReadCurrentHeapProfile()
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "application/octet-stream")
	_ = p.Write(w)
}
