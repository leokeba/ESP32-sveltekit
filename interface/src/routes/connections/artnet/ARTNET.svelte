<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import ArtNET from '~icons/tabler/topology-full-hierarchy';
	import Port from '~icons/tabler/server';

	type ArtNETStatus = {
		enabled: boolean;
		port: number;
	};

	type ArtNETSettings = {
		enabled: boolean;
		port: number;
	};

	let artNetSettings: ArtNETSettings;
	let artNetStatus: ArtNETStatus;

	let formField: any;

	async function getArtNETStatus() {
		try {
			const response = await fetch('/rest/artNetStatus', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			artNetStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return artNetStatus;
	}

	async function getArtNETSettings() {
		try {
			const response = await fetch('/rest/artNetSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			artNetSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return artNetSettings;
	}

	const interval = setInterval(async () => {
		getArtNETStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	onMount(() => {
		if (!$page.data.features.security || $user.admin) {
			getArtNETSettings();
		}
	});

	let formErrors = {
		host: false,
		port: false,
		keep_alive: false,
		topic_length: false
	};

	async function postArtNETSettings(data: ArtNETSettings) {
		try {
			const response = await fetch('/rest/artNetSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status == 200) {
				notifications.success('Security settings updated.', 3000);
				artNetSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function handleSubmitArtNET() {
		let valid = true;

		// Submit JSON to REST API
		if (valid) {
			postArtNETSettings(artNetSettings);
			//alert('Form Valid');
		}
	}
</script>

<SettingsCard collapsible={false}>
	<ArtNET slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">ArtNET</span>
	<div class="w-full overflow-x-auto">
		{#await getArtNETStatus()}
			<Spinner />
		{:then nothing}
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div
						class="mask mask-hexagon h-auto w-10 {artNetStatus.enabled === true
							? 'bg-success'
							: 'bg-error'}"
					>
						<ArtNET
							class="h-auto w-full scale-75 {artNetStatus.enabled === true
								? 'text-success-content'
								: 'text-error-content'}"
						/>
					</div>
					<div>
						<div class="font-bold">Status</div>
						<div class="text-sm opacity-75">
							{#if artNetStatus.enabled}
								Connected
							{:else if !artNetStatus.enabled}
								ArtNET Disabled
							{:else}
								ArtNET Error
							{/if}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Port class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Port</div>
						<div class="text-sm opacity-75">
							{artNetStatus.port}
						</div>
					</div>
				</div>
			</div>
		{/await}
	</div>

	{#if !$page.data.features.security || $user.admin}
		<Collapsible open={false} class="shadow-lg" on:closed={getArtNETSettings}>
			<span slot="title">Change ArtNET Settings</span>

			<form on:submit|preventDefault={handleSubmitArtNET} novalidate bind:this={formField}>
				<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4 sm:grid-cols-2">
					<!-- Enable -->
					<label class="label inline-flex cursor-pointer content-end justify-start gap-4">
						<input
							type="checkbox"
							bind:checked={artNetSettings.enabled}
							class="checkbox checkbox-primary"
						/>
						<span>Enable ArtNET</span>
					</label>
					<div class="hidden sm:block" />
					<!-- URI -->
					<div class="sm:col-span-2">
						<label class="label" for="port">
							<span class="label-text text-md">Port</span>
						</label>
						<input
							type="number"
							min="1"
							max="50000"
							class="input input-bordered invalid:border-error w-full invalid:border-2}"
							bind:value={artNetSettings.port}
							id="port"
							required
						/>
					</div>
				</div>
				<div class="divider mb-2 mt-0" />
				<div class="mx-4 flex flex-wrap justify-end gap-2">
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		</Collapsible>
	{/if}
</SettingsCard>
