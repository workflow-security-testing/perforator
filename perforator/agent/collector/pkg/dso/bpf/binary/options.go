package binary

type ManagerOption func(m *BPFBinaryManager)

func WithAddListeners(listeners ...Listener) ManagerOption {
	return func(m *BPFBinaryManager) {
		m.listeners = append(m.listeners, listeners...)
	}
}
