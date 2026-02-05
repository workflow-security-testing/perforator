package render

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"html/template"
	"io"

	"github.com/yandex/perforator/library/go/core/resource"
)

// WrapJSONInHTMLV2 takes flamegraph JSON data and wraps it in the HTML-v2 template.
// The JSON is gzip-compressed and base64-encoded before embedding.
func WrapJSONInHTMLV2(jsonData []byte, w io.Writer) error {
	// Compress JSON
	buf := new(bytes.Buffer)
	compressor := gzip.NewWriter(buf)
	if _, err := compressor.Write(jsonData); err != nil {
		return err
	}
	if err := compressor.Close(); err != nil {
		return err
	}

	// Prepare template data
	jsCode := template.HTML("<script>" + string(resource.Get("viewer.js")) + "</script>")
	jsonHtml := template.HTML("<script>window.__data__=\"" + base64.StdEncoding.EncodeToString(buf.Bytes()) + "\"</script>")

	return tmpl.ExecuteTemplate(w, string(HTMLFormatV2), &struct {
		Json   template.HTML
		Script template.HTML
	}{
		Json:   jsonHtml,
		Script: jsCode,
	})
}

// RenderJSONAsHTML renders a JSONRenderer's output as HTML-v2.
func RenderJSONAsHTML(renderer JSONRenderer, w io.Writer) error {
	var jsonBuf bytes.Buffer
	if err := renderer.RenderJSON(&jsonBuf); err != nil {
		return err
	}
	return WrapJSONInHTMLV2(jsonBuf.Bytes(), w)
}
