<?php
/**
 * ================================================================= *
 * ECLIPSE / AVIS-DL PURE TRANSPORT LAYER PROTOCOL                   *
 * Compliance: Strict PHP 8.5+                                       *
 * Purpose: Low-level file-state and trigger routing execution loops *
 * ================================================================= *
 */

error_reporting(E_ALL);
ini_set('display_errors', '1');

// Global Endpoint Mappings
define('AVIS_DL_BASE', "https://githubusercontent.com");
define('JOBS_CONFIG_FILE', __DIR__ . '/jobs.json');

include('config.php');

if (!defined('GH_TOKEN')) {
    define('GH_TOKEN', getenv('GH_TOKEN'));
}

// ============================================================================
// CORE TRANSPORT ENGINE (cURL Base)
// ============================================================================

function runGitRequest(string $apiUrl, string $method, ?array $data = null): array 
{
    $ch = curl_init($apiUrl);
    $headers = [
        "Authorization: Bearer " . GH_TOKEN,
        "User-Agent: AVIS-DL-HelloEngine",
        "Accept: application/vnd.github+json",
        "X-GitHub-Api-Version: 2022-11-28"
    ];

    $options = [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_FOLLOWLOCATION => true,
        CURLOPT_TIMEOUT        => 20,
        CURLOPT_HTTPHEADER     => $headers,
        CURLOPT_CUSTOMREQUEST  => $method
    ];

    if ($data !== null) {
        $options[CURLOPT_POSTFIELDS] = json_encode($data);
        $options[CURLOPT_HTTPHEADER][] = 'Content-Type: application/json';
    }

    curl_setopt_array($ch, $options);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

    return [
        'code' => $httpCode,
        'body' => json_decode($response, true) ?: $response
    ];
}

// ============================================================================
// INTERFACE MODULES FOR INDIVIDUAL LINE INVOCATION
// ============================================================================

/**
 * PUSH LINE: Submits structured JSON packages directly to the target leaf nodes.
 */
function pushGitHubFile(string $targetPath, array $jsonData): array 
{
    if (!GH_TOKEN) {
        throw new Exception("GitHub Token parameter verification failed.");
    }

    if (isset($jsonData['archived']) && $jsonData['archived'] === true) {
        return ['code' => 403, 'body' => ['status' => 'error', 'message' => 'Local payload is archived.']];
    }

    $targetUrl = rtrim(GH_API, '/') . '/' . GH_OWNER . '/' . GH_REPO . '/contents/' . ltrim($targetPath, '/');

    $sha = null;
    $lookup = runGitRequest($targetUrl . '?ref=' . GH_BRANCH, 'GET');
    
    if ($lookup['code'] === 200 && isset($lookup['body']['content'])) {
        $sha = $lookup['body']['sha'] ?? null;
        $remoteData = json_decode(base64_decode($lookup['body']['content']), true);

        if (is_array($remoteData) && isset($remoteData['archived']) && $remoteData['archived'] === true) {
            return ['code' => 403, 'body' => ['status' => 'error', 'message' => 'Remote target is archived.']];
        }
    } elseif ($lookup['code'] === 200 && isset($lookup['body']['sha'])) {
        $sha = $lookup['body']['sha'];
    }

    $packet = [
        'message' => 'Automated client transaction push: ' . $targetPath,
        'content' => base64_encode(json_encode($jsonData, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES)),
        'branch'  => GH_BRANCH
    ];

    if ($sha !== null) {
        $packet['sha'] = $sha;
    }

    return runGitRequest($targetUrl, 'PUT', $packet);
}

/**
 * PULL LINE: Fetches raw data blobs directly using the AVIS_DL_BASE protocol layout.
 */
function pullGitHubFile(string $urlPath): array
{
    $absoluteUrl = AVIS_DL_BASE . ltrim($urlPath, '/');
    $ch = curl_init($absoluteUrl);

    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_USERAGENT      => 'AVIS-DL-App/1.0 (mercwar01@gmail.com)',
        CURLOPT_FOLLOWLOCATION => true,
        CURLOPT_TIMEOUT        => 15
    ]);

    $content = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

    return [
        'code' => $httpCode,
        'url'  => $absoluteUrl,
        'raw'  => $content
    ];
}

/**
 * TRIGGER LINE: Dispatches explicit workflow events to your automation runner repository.
 */
function triggerEclipseJob(string $workflowFileName, array $inputs = []): array
{
    if (!GH_TOKEN) {
        throw new Exception("GitHub Token parameter verification failed.");
    }

    $triggerUrl = rtrim(GH_API, '/') . '/' . GH_OWNER . '/' . GH_REPO . '/actions/workflows/' . urlencode($workflowFileName) . '/dispatches';
    
    $packet = [
        'ref'    => GH_BRANCH,
        'inputs' => $inputs
    ];

    return runGitRequest($triggerUrl, 'POST', $packet);
}

/**
 * RETURN RESPONSE LINE: Routes task callbacks directly into context workflows.
 * Explicitly processes internal actions, raw file pulls, and transaction pushes.
 */
function receiveEclipseResponse(array $responseData): array
{
    $context = $responseData['context'] ?? 'action'; // Fallback to raw trigger action
    $jobId = preg_replace('/[^a-zA-Z0-9_\-]/', '', $responseData['job_id'] ?? 'unrouted_task');
    $payload = $responseData['payload'] ?? [];
    
    $logDirectory = __DIR__ . '/logs';
    if (!is_dir($logDirectory)) {
        mkdir($logDirectory, 0755, true);
    }
    
    $logFile = $logDirectory . '/' . $jobId . '_execution.log';
    $logContent = "[" . date('Y-m-d H:i:s') . "] Context: {$context} | Status: " . ($responseData['status'] ?? 'unknown') . "\n";
    $logContent .= "Payload:\n" . print_r($responseData, true) . "\n";
    file_put_contents($logFile, $logContent, FILE_APPEND);

    // Context-specific execution matrix parsing
    switch ($context) {
        case 'push':
            $targetPath = $responseData['target_path'] ?? '';
            if (empty($targetPath)) {
                return ['status' => 'error', 'message' => 'Push path undefined inside response tracking matrix.'];
            }
            return pushGitHubFile($targetPath, $payload);

        case 'pull':
            $targetPath = $responseData['target_path'] ?? '';
            if (empty($targetPath)) {
                return ['status' => 'error', 'message' => 'Pull pointer sequence missing from context wrapper.'];
            }
            return pullGitHubFile($targetPath);

        case 'action':
        default:
            $workflow = $responseData['workflow'] ?? '';
            if (empty($workflow)) {
                return ['status' => 'error', 'message' => 'Target dispatch action file parameter is null.'];
            }
            return triggerEclipseJob($workflow, $payload);
    }
}

// ============================================================================
// MAIN ECLIPSE HUB ROUTER DIRECTIVE
// ============================================================================

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    header('Content-Type: application/json; charset=utf-8');

    if (!file_exists(JOBS_CONFIG_FILE)) {
        echo json_encode(['status' => 'empty', 'jobs' => []]);
        exit;
    }
    
    $configData = json_decode(file_get_contents(JOBS_CONFIG_FILE), true);
    echo json_encode([
        'status'         => 'success',
        'timestamp'      => time(),
        'available_jobs' => $configData['jobs'] ?? []
    ], JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    exit;
}
