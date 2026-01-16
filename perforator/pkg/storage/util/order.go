package util

import (
	"github.com/yandex/perforator/perforator/proto/perforator"
)

type SortColumn struct {
	Name       string
	Descending bool
}

type SortOrder struct {
	Columns []SortColumn
}

func SortOrderFromServicesProto(order *perforator.ListServicesOrderByClause) SortOrder {
	if order == nil {
		return SortOrder{
			Columns: []SortColumn{{Name: "service"}},
		}
	}

	switch *order {
	case perforator.ListServicesOrderByClause_Services:
		return SortOrder{
			Columns: []SortColumn{{Name: "service"}},
		}
	case perforator.ListServicesOrderByClause_ProfileCount:
		return SortOrder{
			Columns: []SortColumn{{Name: "profile_count", Descending: true}},
		}
	default:
		return SortOrder{
			Columns: []SortColumn{{Name: "service"}},
		}
	}
}

func SortOrderFromProto(order *perforator.SortOrder) SortOrder {
	var cols []SortColumn
	for _, c := range order.GetColumns() {
		cols = append(cols, SortColumn{
			Name: c,
			// TODO(PERFORATOR-1068) every column should have its own direction
			Descending: order.GetDirection() == perforator.SortOrder_Descending,
		})
	}
	return SortOrder{
		Columns: cols,
	}
}
