(function() {
    'use strict';

    function closeModal(modalId) {
        const modal = document.getElementById(modalId);
        if (!modal) {
            return;
        }

        modal.setAttribute('aria-hidden', 'true');
    }


    function showModal(modalId) {
        const modal = document.getElementById(modalId);
        if (modal) {
            modal.setAttribute('aria-hidden', 'false');
        }
    }

    function showConfirm(message, callback) {
        const content = document.getElementById('confirm-content');
        const okBtn = document.getElementById('confirm-ok-btn');
        if (!content || !okBtn) {
            return;
        }

        content.textContent = String(message || 'Are you sure?');
        okBtn.onclick = function() {
            callback();
            closeModal('confirm-modal');
        };

        showModal('confirm-modal');
    }

    function updateConnectionStatus() {
        const statusElement = document.getElementById('connection-status');
        const timestamp = document.querySelector('#last-updated [data-timestamp]');

        if (!statusElement) {
            return;
        }

        const label = navigator.onLine ? 'Online' : 'Offline';
        const statusDot = statusElement.querySelector('.status-dot');

        statusElement.className = `status-indicator ${navigator.onLine ? 'status-online' : 'status-offline'}`;

        if (statusDot) {
            statusElement.replaceChildren(statusDot, document.createTextNode(` ${label}`));
        } else {
            statusElement.textContent = label;
        }

        if (timestamp) {
            timestamp.setAttribute('data-timestamp', Date.now());
            timestamp.textContent = 'Just now';
        }
    }

    function wireModalButtons() {
        document.querySelectorAll('[data-close-modal]').forEach(button => {
            button.addEventListener('click', () => {
                closeModal(button.getAttribute('data-close-modal'));
            });
        });
    }

    function wireExternalLinks() {
        document.querySelectorAll('a.external-link').forEach(link => {
            link.addEventListener('click', event => {
                event.preventDefault();
                window.open(link.href, '_blank', 'noopener,noreferrer');
            });
        });
    }

    function wireForgotPassword() {
        const forgotLink = document.querySelector('[data-forgot-password]');
        if (!forgotLink) {
            return;
        }

        forgotLink.addEventListener('click', event => {
            event.preventDefault();
            if (window.EnvyRemote) {
                window.EnvyRemote.showNotification('Password recovery is not implemented in this demo. Please contact your administrator.', 'info');
            }
        });
    }

    function wireLoginForm() {
        const form = document.querySelector('.login-form');
        if (!form) {
            return;
        }

        const submitBtn = form.querySelector('.btn');
        form.addEventListener('submit', function(event) {
            const username = document.getElementById('username');
            const password = document.getElementById('password');
            const usernameValue = username ? username.value.trim() : '';
            const passwordValue = password ? password.value : '';

            if (!usernameValue || !passwordValue) {
                event.preventDefault();
                if (window.EnvyRemote) {
                    window.EnvyRemote.showNotification('Please enter both username and password.', 'error');
                }
                return;
            }

            if (submitBtn) {
                submitBtn.disabled = true;
                submitBtn.textContent = 'Logging in...';
            }
            form.classList.add('submitting');
        });
    }

    function wireActivityFeed() {
        const container = document.getElementById('recent-activity');
        if (!container || !window.EnvyRemote) {
            return;
        }

        function formatTimeAgo(diff) {
            const seconds = Math.floor(diff / 1000);
            const minutes = Math.floor(seconds / 60);
            const hours = Math.floor(minutes / 60);
            const days = Math.floor(hours / 24);
            if (days > 0) return `${days}d ago`;
            if (hours > 0) return `${hours}h ago`;
            if (minutes > 0) return `${minutes}m ago`;
            return `${seconds}s ago`;
        }

        function renderActivities(activities) {
            const body = document.createElement('div');
            body.className = 'card-body';

            if (!Array.isArray(activities) || activities.length === 0) {
                const none = document.createElement('p');
                none.textContent = 'No recent activity.';
                body.appendChild(none);
            } else {
                activities.forEach(activity => {
                    const item = document.createElement('div');
                    item.className = 'activity-item';

                    const time = document.createElement('span');
                    time.className = 'activity-time';
                    time.textContent = formatTimeAgo(Date.now() - Number(activity.timestamp || 0));

                    const text = document.createElement('span');
                    text.className = 'activity-text';
                    text.textContent = String(activity.description || '');

                    item.appendChild(time);
                    item.appendChild(text);
                    body.appendChild(item);
                });
            }

            container.replaceChildren(body);
        }

        function loadRecentActivity() {
            window.EnvyRemote.ajaxRequest('/api/recent-activity')
                .then(data => renderActivities(data.activities))
                .catch(() => renderActivities([]));
        }

        loadRecentActivity();
        const interval = setInterval(loadRecentActivity, 30000);
        window.addEventListener('beforeunload', () => clearInterval(interval));
    }

    window.closeModal = closeModal;
    window.showModal = showModal;
    window.showConfirm = showConfirm;

    document.addEventListener('DOMContentLoaded', function() {
        wireModalButtons();
        wireExternalLinks();
        wireForgotPassword();
        wireLoginForm();
        wireActivityFeed();
        updateConnectionStatus();
        setInterval(updateConnectionStatus, 30000);
        window.addEventListener('online', updateConnectionStatus);
        window.addEventListener('offline', updateConnectionStatus);
    });
})();
