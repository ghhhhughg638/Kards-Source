// Update check: the deployed site publishes version.json ({sha, version}) and changelog.json.
// A page stamped with a different sha than the published one is out of date.

// "2.10.0" > "2.9.1"; unknown or malformed parts count as 0.
export function compareVersions(a, b) {
  const pa = String(a).split(".").map((n) => parseInt(n, 10) || 0),
    pb = String(b).split(".").map((n) => parseInt(n, 10) || 0);
  for (let i = 0; i < Math.max(pa.length, pb.length); i++) {
    const d = (pa[i] || 0) - (pb[i] || 0);
    if (d) return d < 0 ? -1 : 1;
  }
  return 0;
}

// Placeholders are only replaced by the deploy workflow; local copies never prompt.
export const isStamped = (build) =>
  typeof build?.sha === "string" && !build.sha.startsWith("__") && build.sha.length > 0;

export function isOutdated(build, latest) {
  return isStamped(build) && typeof latest?.sha === "string" && latest.sha !== build.sha;
}

// Changelog entries (newest first) that the user does not have yet.
export function entriesSince(changelog, currentVersion, limit = 6) {
  if (!Array.isArray(changelog)) return [];
  const fresh = changelog.filter(
    (e) => e && typeof e.version === "string" && compareVersions(e.version, currentVersion) > 0,
  );
  return (fresh.length ? fresh : changelog.slice(0, 1)).slice(0, limit);
}
