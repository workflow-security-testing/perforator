ALTER TABLE profiles
ADD COLUMN IF NOT EXISTS custom_profiling_operation_id String CODEC(ZSTD(1)),
ADD INDEX IF NOT EXISTS idx_custom_profiling_operation_id custom_profiling_operation_id TYPE minmax GRANULARITY 1
