ALTER TABLE profiles
DROP INDEX IF EXISTS idx_custom_profiling_operation_id,
DROP COLUMN IF EXISTS custom_profiling_operation_id
